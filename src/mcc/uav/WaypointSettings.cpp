#include "mcc/uav/WaypointSettings.h"
#include "mcc/ui/CoordinateSystemController.h"
#include "mcc/ui/Settings.h"
#include "mcc/uav/Structs.h"
#include "mcc/uav/Uav.h"
#include "mcc/uav/UavController.h"
#include "mcc/ui/LatLonEditor.h"
#include "mcc/ui/FastEditDoubleSpinBox.h"
#include "mcc/uav/Route.h"
#include "mcc/geo/Geod.h"
#include "mcc/geo/Coordinate.h"
#include "mcc/geo/Constants.h"
#include "mcc/res/Resource.h"

#include <bmcl/Rc.h>
#include <bmcl/MakeRc.h>

#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QToolButton>
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QRadioButton>
#include <QStackedWidget>
#include <QApplication>
#include <QMouseEvent>
#include <QMenu>
#include <QMetaObject>
#include <QDebug>

Q_DECLARE_METATYPE(mccmsg::Property);

namespace mccuav {

WaypointSettings::WaypointSettings(mccuav::Uav* device,
                                   mccuav::Route* route,
                                   const mccui::CoordinateSystemController* csController,
                                   mccui::Settings* settings,
                                   QWidget* parent)
    : mccui::Dialog(parent)
    , _speedToAll(new QPushButton("для всех"))
    , _heightTypeBox(nullptr)
    , _heightToAll(new QPushButton("для всех"))
    , _sleepBox(new mccui::FastEditDoubleSpinBox())
    , _sleepUnits(nullptr)
    , _uav(device)
    , _route(route)
    , _idx(0)
    , _updateFlags(false)
    , _csController(csController)
    , _settings(settings)
    , _aerobotMode(settings->read("ide/aerobotMode", false).toBool())
{
    setModal(false);
    connect(_route, &mccuav::Route::waypointChanged, this,
            [this](const mccmsg::Waypoint&, int index)
            {
                if (_idx == index)
                {
                    set(index);
                }
            }
    );

    connect(_route, &mccuav::Route::allWaypointsChanged, this,
            [this]()
    {
        //TODO: multiple selection
        bmcl::Option<size_t> selectedPoint;
        if (!_route->selectedPointIndexes().empty())
            selectedPoint = _route->selectedPointIndexes().front();

        if(selectedPoint.isSome() && !_route->isEmpty())
        {
            int index = selectedPoint.unwrap();
            if (index >= _route->waypointsCount())
                index = _route->waypointsCount() - 1;
            set(index);
        }
    });

    connect(_route, &mccuav::Route::waypointMoved, this, [this](int from, int to) {
        (void)from; set(to);
    });

    connect(_route, &mccuav::Route::waypointRemoved, this, [this](int index) {
        if (index >= _route->waypointsCount())
            set(_route->waypointsCount() - 1);
        else
            set(index);
    });

    connect(_route, &mccuav::Route::selectedWaypointsChanged, this,
            [this]()
            {
                const std::vector<size_t>& indexes = _route->selectedPointIndexes();
                bmcl::Option<size_t> index;
                if (!indexes.empty())
                    index = indexes.front();

                if (index.isNone())
                {
                    setEnabled(false);
                    return;
                }

                setEnabled(true);
                set(static_cast<int>(index.unwrap()));
            }
    );

    auto mainLayout = new QVBoxLayout;

    auto buttonLayout = new QHBoxLayout;
    _addButton = new QPushButton(mccres::loadIcon(mccres::ResourceKind::AddButtonIcon), "Добавить");
    _removeButton = new QPushButton(mccres::loadIcon(mccres::ResourceKind::DeleteButtonIcon), "Удалить");
    _uploadButton = new QPushButton(QIcon(":/transfer.png"), "На борт");
    _okButton = new QPushButton(mccres::loadIcon(mccres::ResourceKind::OkButtonIcon), "Ok");

    buttonLayout->addWidget(_addButton);
    buttonLayout->addWidget(_removeButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(_uploadButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(_okButton);

    auto paramsGroupBox = new QGroupBox("Параметры");
    auto paramsLayout = new QGridLayout;
    paramsGroupBox->setLayout(paramsLayout);
    const int labelColumn = 1;
    paramsLayout->setColumnStretch(labelColumn + 4, 1);
    paramsLayout->addWidget(new QLabel("Скорость"), 0, labelColumn);
    paramsLayout->addWidget(new QLabel("Высота"), 1, labelColumn);
//    paramsLayout->addWidget(new QLabel("Высота задается"), 2, 0);

    _speedBox = new mccui::FastEditDoubleSpinBox();
    _speedBox->setMinimum(0);
    _speedBox->setMaximum(2000);
    _speedBox->setDecimals(2);

    _speedUnits = new QComboBox();
    _speedUnits->addItem(" м/с");

    _speedToAll->setToolTip("Задать значение для всех точек маршрута");
    connect(_speedToAll, &QPushButton::clicked, this,
            [this]()
    {
        double wantedSpeed = _speedBox->value();
        for(int i = 0; i < _route->waypointsCount(); ++i) {
            mccmsg::Waypoint wp = _route->waypointAt(i);
            wp.speed = wantedSpeed;
            _route->setWaypoint(wp, i);
        }
    });

    paramsLayout->addWidget(_speedBox, 0, labelColumn + 1);
    paramsLayout->addWidget(_speedUnits, 0, labelColumn + 2);
    paramsLayout->addWidget(_speedToAll, 0, labelColumn + 3);

    _heightBox = new mccui::FastEditDoubleSpinBox();
    _heightBox->setMinimum(-6378100.0);
    _heightBox->setMaximum(10000000.0);
    _heightBox->setDecimals(2);

    //FIXME: использовать текущие единцы CoordinateSystemController
    _heightUnits = new QComboBox();
    _heightUnits->addItem(" м");

    _heightToAll->setToolTip(_speedToAll->toolTip());
    connect(_heightToAll, &QPushButton::clicked, this,
            [this]()
    {
        double inverseAlt = _heightBox->value();
        const mccgeo::CoordinateConverter* conv = _csController->currentConverter();
        for(int i = 0; i < _route->waypointsCount(); ++i) {
            mccmsg::Waypoint wp = _route->waypointAt(i);
            mccgeo::Position inverse = conv->convertForward(wp.position).position();
            inverse.setAltitude(inverseAlt);
            wp.position = conv->convertInverse(inverse).position();
            _route->setWaypoint(wp, i);
        }
    });

    paramsLayout->addWidget(_heightBox, 1, labelColumn + 1);
    paramsLayout->addWidget(_heightUnits, 1, labelColumn + 2);
    paramsLayout->addWidget(_heightToAll, 1, labelColumn + 3);

//    _heightTypeBox = new QComboBox;
//    _heightTypeBox->addItem("Относительно эллипсоида");
//    _heightTypeBox->addItem("Относительно местности");
//    paramsLayout->addWidget(_heightTypeBox , 2, 1);

    if (_aerobotMode)
    {
        _sleepBox->setMinimum(0);
        _sleepBox->setMaximum(24*60*60);
        _sleepBox->setDecimals(0);

        _sleepUnits = new QComboBox();
        _sleepUnits->addItem(" с");

        QLabel *icon(new QLabel());
        icon->setPixmap(QPixmap(":/resources/waypoints/pause.svg"));
        paramsLayout->addWidget(icon, 2, 0);

        paramsLayout->addWidget(new QLabel("Пауза"), 2, labelColumn);
        paramsLayout->addWidget(_sleepBox, 2, labelColumn + 1);
        paramsLayout->addWidget(_sleepUnits, 2, labelColumn + 2);
    }
    else
    {
        _sleepBox->hide();
    }

    _flagsGroupBox = new QGroupBox("Свойства точки");
    _flagsLayout = new QGridLayout();
    _flagsGroupBox->setLayout(_flagsLayout);
    _addPropertyButton = new QPushButton("Добавить свойство");
    _addPropertyMenu = new QMenu();
    _addPropertyButton->setMenu(_addPropertyMenu);

//     _checkBoxReadOnly    = new QCheckBox("Только для чтения");
//     _checkBoxTarget      = new QCheckBox("Цель");
//     _checkBoxParashute   = new QCheckBox("Парашют");
//     _checkBoxTurnBack    = new QCheckBox("Разворот назад");
//     _checkBoxTurnForward = new QCheckBox("Разворот вперед");
//     _checkBoxWaiting     = new QCheckBox("Ожидание");
//     _checkBoxSwitchRoute = new QCheckBox("Переход");
//     _checkBoxHome        = new QCheckBox("Дом");
//     _checkBoxLanding     = new QCheckBox("Посадка");

    _noModeButton = new QRadioButton("Без изменения");
    _reynoldsModeButton = new QRadioButton("Рейнольдс");
    _formationModeButton = new QRadioButton("Формация");
    _snakeModeButton = new QRadioButton("Змейка");
    _loopModeButton = new QRadioButton("Петля");

    _modeDetailsWidget = new QStackedWidget();

    QLabel* reynoldsDetails = new QLabel("Режим полета: Рейнольдс");
    QLabel* formationDetails = new QLabel("Режим полета: Формация. Редактирование формации осуществляется в окне «Формация»");
    formationDetails->setWordWrap(true);
    QLabel* snakeDetails = new QLabel("Режим полета: Змейка");

    QWidget* loopDetails = new QWidget();
    QGridLayout* loopLayout = new QGridLayout();
    loopDetails->setLayout(loopLayout);
    QLabel* loopDetailsLabel = new QLabel("Режим полета: Петля");
    loopDetailsLabel->setWordWrap(true);
    loopLayout->addWidget(loopDetailsLabel, 0, 0, 1, 2);
    loopLayout->addWidget(new QLabel("Радиус:"), 1, 0);

    _loopRadius = new mccui::FastEditDoubleSpinBox();
    _loopRadius->setValue(7.0);
    loopLayout->addWidget(_loopRadius, 1, 1);

    _modeDetailsWidget->addWidget(new QWidget());
    _modeDetailsWidget->addWidget(reynoldsDetails);
    _modeDetailsWidget->addWidget(formationDetails);
    _modeDetailsWidget->addWidget(snakeDetails);
    _modeDetailsWidget->addWidget(loopDetails);

    _modeGroup = new QButtonGroup();
    _modeGroup->addButton(_noModeButton);
    _modeGroup->addButton(_reynoldsModeButton);
    _modeGroup->addButton(_formationModeButton);
    _modeGroup->addButton(_snakeModeButton);
    _modeGroup->addButton(_loopModeButton);

    connect(_modeGroup, static_cast<void(QButtonGroup::*)(QAbstractButton *, bool)>(&QButtonGroup::buttonToggled),
            [this](QAbstractButton *button, bool checked)
            {
                (void)button;
                if(checked)
                    apply();
            }
    );
    //connect(_noModeButton, &QRadioButton::pressed, this, &WaypointSettings::apply);
    //connect(_reynoldsModeButton, &QRadioButton::pressed, this, &WaypointSettings::apply);
    //connect(_formationModeButton, &QRadioButton::pressed, this, &WaypointSettings::apply);
    //connect(_snakeModeButton, &QRadioButton::pressed, this, &WaypointSettings::apply);
    //connect(_loopModeButton, &QRadioButton::pressed, this, &WaypointSettings::apply);

//     auto addWidgetWithIcon = [flagsLayout](QWidget *w, int row, int column, const QString& path = QString())
//     {
//         flagsLayout->addWidget(w, row, column + 1);
// 
//         if(path.isNull())
//             return;
//         QLabel *icon(new QLabel());
//         icon->setPixmap(QPixmap(path));
//         flagsLayout->addWidget(icon, row, column);
//     };

//     if (_aerobotMode)
//     {
//         flagsLayout->addWidget(_noModeButton,      0, 1);
// 
//         addWidgetWithIcon(_reynoldsModeButton,     1, 1, ":/resources/waypoints/reynolds.svg");
//         addWidgetWithIcon(_formationModeButton,    2, 1, ":/resources/waypoints/formation.svg");
//         addWidgetWithIcon(_snakeModeButton,        3, 1, ":/resources/waypoints/snake.svg");
//         addWidgetWithIcon(_loopModeButton,         4, 1, ":/resources/waypoints/loop.svg");
// 
//         flagsLayout->addWidget(_modeDetailsWidget, 0, 2, flagsLayout->rowCount(), 1);
// 
//         // Target
//         QFrame *line = new QFrame();
//         line->setFrameShape(QFrame::HLine);
//         line->setStyleSheet("QFrame[frameShape=\"4\"]" // QFrame::HLine == 0x0004
//                             "{color: darkgray;}");
//         flagsLayout->addWidget(line,               5, 0, 1, 3);
// 
//         //addWidgetWithIcon(_checkBoxTarget,         6, 1, ":/resources/waypoints/target2.svg");
//     }
//     else
//     {
//         flagsLayout->setColumnStretch(0, 1);
//         flagsLayout->setColumnStretch(6, 1);
// 
//         bool isSimple = settings->read("ide/simpleMode", false).toBool();
//         if (isSimple)
//         {
// //             addWidgetWithIcon(_checkBoxTarget,          0, 1, ":/resources/waypoints/target2.svg");
// //             addWidgetWithIcon(_checkBoxSwitchRoute,     1, 1, ":/resources/waypoints/switch_route.svg");
// //             addWidgetWithIcon(_checkBoxLanding,         2, 1, ":/resources/waypoints/landing.svg");
//         }
//         else
//         {
// //             addWidgetWithIcon(_checkBoxReadOnly,        0, 1, ":/resources/waypoints/readonly.svg");
// //             addWidgetWithIcon(_checkBoxTarget,          1, 1, ":/resources/waypoints/target2.svg");
// //             addWidgetWithIcon(_checkBoxParashute,       2, 1, ":/resources/waypoints/parachute.svg");
// //             addWidgetWithIcon(_checkBoxTurnBack,        3, 1, ":/resources/waypoints/turn_back.svg");
// //             addWidgetWithIcon(_checkBoxTurnForward,     4, 1, ":/resources/waypoints/turn_forward.svg");
// // 
// //             addWidgetWithIcon(_checkBoxWaiting,         0, 4, ":/resources/waypoints/pause.svg");
// //             addWidgetWithIcon(_checkBoxSwitchRoute,     1, 4, ":/resources/waypoints/switch_route.svg");
// //             addWidgetWithIcon(_checkBoxHome,            2, 4, ":/resources/waypoints/home.svg");
// //             addWidgetWithIcon(_checkBoxLanding,         3, 4, ":/resources/waypoints/landing.svg");
// 
//             QFrame* line = new QFrame();
//             line->setFrameShape(QFrame::VLine);
//             line->setFrameShadow(QFrame::Sunken);
//             flagsLayout->addWidget(line,               0, 3, 5, 1);
//         }
//     }

    auto infoLayout = new QGridLayout;
    QToolButton* btnPrev = new QToolButton();
    btnPrev->setText("<");

    QToolButton* btnNext = new QToolButton();
    btnNext->setText(">");

    connect(btnPrev, &QToolButton::pressed, this,
            [this]()
            {
                apply();

                int count = _route->waypointsCount();
                if (--_idx < 0)
                    _idx = count - 1;
                _route->setSelectedPoint(_idx);
            }
    );

    connect(btnNext, &QToolButton::pressed, this,
            [this]()
            {
                apply();

                int count = _route->waypointsCount();
                if (++_idx >= count)
                    _idx = 0;
                _route->setSelectedPoint(_idx);
            }
    );

    _routeName = new QLabel();
    _pointIndex = new QLabel();

    infoLayout->addWidget(new QLabel("Маршрут: "), 0, 0);
    infoLayout->addWidget(_routeName, 0, 1);
    infoLayout->setColumnStretch(1, 1);
    infoLayout->addWidget(new QLabel("Точка: "), 0, 2);
    infoLayout->addWidget(btnPrev, 0, 3);
    infoLayout->addWidget(_pointIndex, 0, 4);
    infoLayout->addWidget(btnNext,0, 5);

    _latLonEditor = new mccui::LatLonEditor(csController);
    infoLayout->addWidget(_latLonEditor, 1, 0, 1, 6);

    mainLayout->addLayout(infoLayout);
    mainLayout->addWidget(paramsGroupBox);

    auto addPropertyLayout = new QHBoxLayout();
    addPropertyLayout->addWidget(new QLabel("Добавить свойство:"));
    addPropertyLayout->addStretch();
    addPropertyLayout->addWidget(_addPropertyButton);

    mainLayout->addLayout(addPropertyLayout);
    mainLayout->addWidget(_flagsGroupBox);

    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);

    connect(_addButton, &QPushButton::clicked, this, [this]() {
        apply();
        if (!_route->canInsertWaypoint())
            return;
        mccgeo::LatLon latLon;
        const mccmsg::Waypoints& lst = _route->waypointsList();
        int count = _route->waypointsCount();
        int offset = 40;
        if(count == 0)
        {
            latLon = initialLatLon();
        }
        else if(count == 1)
        {
            latLon = latLonOnLine(lst.front().position, 0, offset);
        }
        else
        {
            double angle =angleBetween(lst[lst.size() - 2].position, lst.back().position);
            latLon = latLonOnLine(lst.back().position, angle, offset);
        }
        mccmsg::Waypoint wp;
        wp.position.latitude() = latLon.latitude();
        wp.position.longitude() = latLon.longitude();
        if(!lst.empty())
        {
            const auto& lastWp = lst.back();
            wp.position.altitude() = lastWp.position.altitude();
            wp.speed = lastWp.speed;
        }
        else
        {
            wp.position.altitude() = _uav->uavController()->calcWaypointAltitudeAt(latLon);
        }
        _route->addWaypoint(wp);
        _route->setSelectedPoint(_route->waypointsCount() - 1);
    }
    );

    connect(_removeButton, &QPushButton::clicked, this,
            [this]()
    {
        _route->removeWaypoint(_idx);
        _route->setSelectedPoint(_idx);

        if(_route->waypointsCount() == 0)
            this->close();
    });

    connect(_uploadButton, &QPushButton::clicked, this, [this]() {
        _uav->uploadEditableRoute();
        }
    );
    connect(_okButton, &QPushButton::clicked, this, [this]()
    {
        accept();
    });

//     connect(_checkBoxReadOnly, &QCheckBox::stateChanged, this, &WaypointSettings::apply);
//     connect(_checkBoxTarget, &QCheckBox::stateChanged, this, &WaypointSettings::apply);
//     connect(_checkBoxParashute, &QCheckBox::stateChanged, this, &WaypointSettings::apply);
//     connect(_checkBoxTurnBack, &QCheckBox::stateChanged, this, &WaypointSettings::apply);
//     connect(_checkBoxTurnForward, &QCheckBox::stateChanged, this, &WaypointSettings::apply);
//     connect(_checkBoxWaiting, &QCheckBox::stateChanged, this, &WaypointSettings::apply);
//     connect(_checkBoxSwitchRoute, &QCheckBox::stateChanged, this, &WaypointSettings::apply);
//     connect(_checkBoxHome, &QCheckBox::stateChanged, this, &WaypointSettings::apply);
//     connect(_checkBoxLanding, &QCheckBox::stateChanged, this, &WaypointSettings::apply);
    connect(_latLonEditor, &mccui::LatLonEditor::valueChanged, this, &WaypointSettings::apply);
    connect(_latLonEditor, &mccui::LatLonEditor::systemChanged, this, &WaypointSettings::setCorrectHeight);

    connect<void (mccui::FastEditDoubleSpinBox::*)(double)>(_heightBox, &mccui::FastEditDoubleSpinBox::valueChanged, this, &WaypointSettings::apply);
    connect<void (mccui::FastEditDoubleSpinBox::*)(double)>(_speedBox, &mccui::FastEditDoubleSpinBox::valueChanged, this, &WaypointSettings::apply);
    connect<void (mccui::FastEditDoubleSpinBox::*)(double)>(_sleepBox, &mccui::FastEditDoubleSpinBox::valueChanged, this, &WaypointSettings::apply);
    connect<void (mccui::FastEditDoubleSpinBox::*)(double)>(_loopRadius, &mccui::FastEditDoubleSpinBox::valueChanged, this, &WaypointSettings::apply);

    adjustSize();
    setFixedSize(size());

    _okButton->setDefault(true);
}

WaypointSettings::~WaypointSettings()
{
    delete _latLonEditor;
}

void WaypointSettings::set(int inx)
{
    _updateFlags = false;
    if (_route->waypointsCount() == 0 || inx > _route->waypointsCount())
    {
        setEnabled(false);
        return;
    }
    _idx = inx;

    setEnabled(true);

    const mccmsg::Waypoint& wp = _route->waypointAt(inx);

    _routeName->setText(QString("<b>%1</b>").arg(_route->name()));
    _pointIndex->setText(QString("<b>%1</b>").arg(inx + 1));

    _latLonEditor->setLatLon(wp.position.latLon());
    
    updateProperties();

    setCorrectHeight();

    if(_heightTypeBox != nullptr) _heightTypeBox->setCurrentIndex(0);
    _speedBox->setValue(wp.speed);
//     _checkBoxReadOnly->setChecked(wp.properties.testFlag(WaypointType::ReadOnly));
//     _checkBoxTarget->setChecked(wp.properties.testFlag(WaypointType::Target));
//     _checkBoxParashute->setChecked(wp.properties.testFlag(WaypointType::Parashute));
//     _checkBoxTurnBack->setChecked(wp.properties.testFlag(WaypointType::TurnBack));
//     _checkBoxTurnForward->setChecked(wp.properties.testFlag(WaypointType::TurnForward));
//     _checkBoxWaiting->setChecked(wp.properties.testFlag(WaypointType::Waiting));
//     _checkBoxSwitchRoute->setChecked(wp.properties.testFlag(WaypointType::SwitchRoute));
//     _checkBoxHome->setChecked(wp.properties.testFlag(WaypointType::Home));
//     _checkBoxLanding->setChecked(wp.properties.testFlag(WaypointType::Landing));

//     _noModeButton->setChecked(wp.properties.testFlag(WaypointType::Normal));
//     _reynoldsModeButton->setChecked(wp.properties.testFlag(WaypointType::Reynolds));
//     _formationModeButton->setChecked(wp.properties.testFlag(WaypointType::Formation));
//     _snakeModeButton->setChecked(wp.properties.testFlag(WaypointType::Snake));
//     _loopModeButton->setChecked(wp.properties.testFlag(WaypointType::Loop));

    if (_noModeButton->isChecked())
    {
        _modeDetailsWidget->setCurrentIndex(0);
    }
    if (_reynoldsModeButton->isChecked())
    {
        _modeDetailsWidget->setCurrentIndex(1);
    }
    if (_formationModeButton->isChecked())
    {
        _modeDetailsWidget->setCurrentIndex(2);
    }
    if (_snakeModeButton->isChecked())
    {
        _modeDetailsWidget->setCurrentIndex(3);
    }
    if (_loopModeButton->isChecked())
    {
        _modeDetailsWidget->setCurrentIndex(4);
    }

//     if (wp.sleep.isNone())
//     {
//         _sleepBox->setValue(0);
//     }
//     else
//     {
//         _sleepBox->setValue(wp.sleep.unwrap());
//     }

//     if (wp.loopRadius.isNone())
//     {
//         _loopRadius->setValue(7.0);
//     }
//     else
//     {
//         _loopRadius->setValue(wp.loopRadius.unwrap());
//     }

    _updateFlags = true;
}


void WaypointSettings::setEditMode(mccuav::Route::EditMode editMode)
{
    using mccuav::Route;
    using mccui::LatLonEditor;

    switch (editMode)
    {
    case Route::EditMode::Latitude:
        _latLonEditor->setEditMode(LatLonEditor::EditMode::Latitude);
        break;
    case Route::EditMode::Longitude:
        _latLonEditor->setEditMode(LatLonEditor::EditMode::Longitude);
        break;
    case Route::EditMode::Altitude:
        _heightBox->setFocus();
        break;
    case Route::EditMode::Speed:
        _speedBox->setFocus();
        break;
    case Route::EditMode::Flags:
        _flagsGroupBox->setFocus();
        break;
    }
}

void WaypointSettings::apply()
{
    if (!_updateFlags)
        return;
//    QWidget *focused = QApplication::focusWidget();

    mccmsg::Waypoint wp = _route->waypointAt(_idx);

    double alt = _heightBox->value();

    wp.position = _latLonEditor->positionWithConvertedAlt(alt);
    wp.speed = _speedBox->value();
    for(const auto& i: _propertiesEditors)
    {
        wp.properties.set(i->get());
    }
    _route->setWaypoint(wp, _idx);

//     if(isVisible() && focused != nullptr)
//     {
//         focused->setFocus(Qt::MouseFocusReason);
//     }
}

void WaypointSettings::setCorrectHeight()
{
    if(_idx >= _route->waypointsCount())
        return;

    const mccgeo::CoordinateConverter* conv = _csController->currentConverter();
    const mccgeo::Position& pos = _route->waypointAt(_idx).position;
    double alt = conv->convertForward(pos).z();
    _heightBox->setValue(alt);
}

void WaypointSettings::updateAvailableFlags()
{
    _addPropertyMenu->clear();
    assert(_uav);
    assert(_uav->firmwareDescription().isSome());
    const auto& properties = _uav->firmwareDescription()->frm()->optional();
    for (const auto& p : properties)
    {
        auto addAction = _addPropertyMenu->addAction(QString::fromStdString(p->info()));
        addAction->setData(QVariant::fromValue(p->name()));
        connect(addAction, &QAction::triggered, this, &WaypointSettings::addPropertyClicked);
    }
}

void WaypointSettings::updateProperties()
{
    const auto& wp = _route->waypointAt(_idx);
    //cleanup properties
    _propertiesEditors.clear();
    _flagsGroupBox->setVisible(!wp.properties.values().empty());
    int row = 0;

    for (const auto& p : wp.properties.values())
    {
        auto editorCreator = p->property()->editor();
        assert(editorCreator);
        auto editor = editorCreator(p->property());
        _flagsLayout->addWidget(editor->widget(), row, 0);
        if (_uav && _uav->firmwareDescription().isSome() && !_uav->firmwareDescription()->frm().isNull())
        {
            auto frm = _uav->firmwareDescription()->frm();
            const auto& reqs = frm->required();
            const auto i = std::find_if(reqs.begin(), reqs.end(), [&p](const auto& r) { return p->property()->name() == r->name(); });
            if (i == reqs.end())
            {
                auto removeButton = new QPushButton("Удалить");
                connect(removeButton, &QPushButton::clicked, this, &WaypointSettings::removePropertyClicked);
                removeButton->setProperty("property-name", QVariant::fromValue(p->property()->name()));
                _flagsLayout->addWidget(removeButton, row, 1, Qt::AlignRight);
            }
        }

        row++;
        editor->set(p);
        auto changedCbk = [this]() { apply(); };
        editor->setValueChangedCallback(changedCbk);

        _propertiesEditors.emplace_back(bmcl::wrapRc<mccmsg::PropertyEditor>(editor));
    }

}

void WaypointSettings::addPropertyClicked()
{
    auto self = qobject_cast<QAction*>(sender());
    if (!self->data().isValid())
    {
        assert(false);
        return;
    }
    auto property = self->data().value<mccmsg::Property>();
    mccmsg::Waypoint& wp = _route->waypointAt(_idx);

    if (_uav && _uav->firmwareDescription().isSome() && !_uav->firmwareDescription()->frm().isNull())
    {
        const auto& ps = _uav->firmwareDescription()->frm()->optional();
        const auto i = std::find_if(ps.begin(), ps.end(), [property](const auto& p) { return p->name() == property; });
        if (i != ps.end())
        {
            if (wp.properties.add((*i)->defaultValue()))
                updateProperties();
        }
    }
}

void WaypointSettings::removePropertyClicked()
{
     auto self = qobject_cast<QPushButton*>(sender());
     if (!self->property("property-name").isValid())
     {
         assert(false);
         return;
     }
     auto property = self->property("property-name").value<mccmsg::Property>();
     mccmsg::Waypoint& wp = _route->waypointAt(_idx);
     if (wp.properties.remove(property))
         updateProperties();
}

void WaypointSettings::showEvent(QShowEvent* event)
{
    _latLonEditor->resetFormat();
    updateAvailableFlags();
    QWidget::showEvent(event);
}

void WaypointSettings::closeEvent(QCloseEvent* event)
{
    apply();

    QWidget::closeEvent(event);
}

}
