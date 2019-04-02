#include "RouteEditorTool.h"
#include "RouteListTreeView.h"
#include "RouteListModel.h"
#include "RouteListFilterModel.h"
#include "MissionPlanModel.h"
#include "MissionPlanDelegate.h"
#include "TableViewWithFreezeSelection.h"
#include "ScanAreaDialog.h"

#include "mcc/res/Resource.h"
#include "mcc/ui/ColorDialogOptions.h"
#include "mcc/uav/Route.h"
#include "mcc/uav/RoutesController.h"
#include "mcc/uav/UavController.h"

#include <QMenu>
#include <QIcon>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <QTableView>
#include <QHeaderView>
#include <QMessageBox>
#include <QInputDialog>
#include <QColorDialog>
#include <QFileDialog>
#include <QDebug>

class SelectionColorKillerDelegate : public QItemDelegate
{
public:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        QStyleOptionViewItem myOption = option;
        myOption.state &= (~QStyle::State_Selected);
        QItemDelegate::paint(painter, myOption, index);
    }
};

class QObjectSignalBlocker
{
public:
    QObjectSignalBlocker(QObject* target)
        : _target(target)
    {
        _target->blockSignals(true);
    }
    ~QObjectSignalBlocker()
    {
        _target->blockSignals(false);
    }
private:
    QObject* _target;
};

constexpr const char* routesDirKey = "ide/routesDir";

void addMenuAction(QMenu* menu, const QString& text, const std::function<void()>& func)
{
    auto action = menu->addAction(text);
    QObject::connect(action, &QAction::triggered, func);
}

RoutesListWidget::RoutesListWidget(QWidget* parent, mccui::Settings* settings, mccuav::UavController* uavController, mccuav::RoutesController* routesController, mccmsg::ProtocolController* pController)
        : QWidget(parent)
        , _routesDirWriter(settings->acquireSharedWriter(routesDirKey).unwrap())
        , _uavController(uavController)
        , _routesController(routesController)
        , _pController(pController)
{
    _editRouteButton = new QPushButton("Редактирование");
    _editRouteButton->setCheckable(true);
    _editRouteButton->setStyleSheet("QPushButton:checked { background-color: rgb(85, 255, 127); }");
    _copyRouteButton = new QPushButton();
    _clearRouteButton = new QPushButton();
    _uploadRouteButton = new QPushButton();
    _saveRouteButton = new QPushButton();
    _loadRouteButton = new QPushButton();

    _copyRouteButton->setFixedSize(25, 25);
    _clearRouteButton->setFixedSize(25, 25);
    _uploadRouteButton->setFixedSize(25, 25);
    _saveRouteButton->setFixedSize(25, 25);
    _loadRouteButton->setFixedSize(25, 25);

    _copyRouteButton->setIcon(mccres::loadIcon(mccres::ResourceKind::ResetButtonIcon));
    _clearRouteButton->setIcon(mccres::loadIcon(mccres::ResourceKind::ClearButtonIcon));
    _uploadRouteButton->setIcon(QIcon(":/transfer.png"));
    _saveRouteButton->setIcon(QIcon(":/save.png"));
    _loadRouteButton->setIcon(QIcon(":/load.png"));

    _uploadRouteButton->setToolTip("Сохранить маршрут на борт");
    _copyRouteButton->setToolTip("Отменить несохраненные изменения маршрута");
    _clearRouteButton->setToolTip("Очистить маршрут");
    _saveRouteButton->setToolTip("Сохранить маршрут в файл");
    _loadRouteButton->setToolTip("Загрузить маршрут из файла");

    connect(_copyRouteButton, &QPushButton::pressed, this, &RoutesListWidget::copyRouteButtonClicked);
    connect(_clearRouteButton, &QPushButton::pressed, this, &RoutesListWidget::clearRouteButtonClicked);
    connect(_uploadRouteButton, &QPushButton::pressed, this, &RoutesListWidget::uploadRouteButtonClicked);
    connect(_saveRouteButton, &QPushButton::pressed, this, &RoutesListWidget::saveRouteButtonClicked);
    connect(_loadRouteButton, &QPushButton::pressed, this, &RoutesListWidget::loadRouteButtonClicked);

    auto buttonsLayout = new QHBoxLayout();
    buttonsLayout->addWidget(_editRouteButton);
    buttonsLayout->addWidget(_copyRouteButton);
    buttonsLayout->addWidget(_clearRouteButton);
    buttonsLayout->addWidget(_uploadRouteButton);
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(_saveRouteButton);
    buttonsLayout->addWidget(_loadRouteButton);
    buttonsLayout->setContentsMargins(0, 0, 0, 0);

    _routesView = new RouteListTreeView(this);
    _routesView->setRootIsDecorated(false);
    auto layout = new QVBoxLayout();
    layout->addLayout(buttonsLayout);
    layout->addWidget(_routesView);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    _model = new RouteListModel(this);
    _filterModel = new RouteListFilterModel(this);
    _filterModel->setSourceModel(_model);
    _routesView->setModel(_filterModel);

    _routesView->hideColumn(1);
    _routesView->setColumnWidth(0, 30);
    _routesView->header()->setStretchLastSection(true);
    _routesView->setItemDelegateForColumn(0, new SelectionColorKillerDelegate());

    _routesView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(_routesView, &QWidget::customContextMenuRequested, this, &RoutesListWidget::contextMenuRequested);
    connect(_routesController, &mccuav::RoutesController::selectedRouteChanged, this, &RoutesListWidget::setRoute);
    connect(_uavController, &mccuav::UavController::selectionChanged, this, [this](mccuav::Uav* uav) {setRoute(nullptr, uav); });
    connect(_routesController, &mccuav::RoutesController::routeEditingChanged, this,
            [this](bool isEditing)
            {
                _editRouteButton->setChecked(isEditing);
                _routesView->setVisible(!isEditing);
                updateButtons(_routesController->selectedRoute());
            }
    );

    connect(_editRouteButton, &QPushButton::clicked, this,
            [this](bool isChecked)
            {
                if (isChecked)
                    _routesController->startEditRoute();
                else
                    _routesController->stopEditRoute();
            }
    );

    connect(_routesView, &RouteListTreeView::clearRouteSelection, this,
            [this]()
            {
                _routesController->selectRoute(nullptr);
            }
    );

    connect(_routesView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &RoutesListWidget::selectRoute);
    connect(_routesView, &QTreeView::doubleClicked, this, &RoutesListWidget::routeDoubleClicked);
    setEmptyUav();
}

void RoutesListWidget::setEmptyUav()
{
    setRoute(nullptr, nullptr);
}

void RoutesListWidget::setRoute(mccuav::Route* route, mccuav::Uav* uav)
{
    if (!uav)
    {
        _model->setEmptyDevice();
        setEnabled(false);
        return;
    }

    setEnabled(true);
    _model->setDevice(uav);
    _routesView->resizeColumnToContents(0);
    _routesView->setColumnWidth(1, 50);
    updateButtons(route);
}

void RoutesListWidget::updateButtons(mccuav::Route* route)
{
    bool editMode = _routesController->isEditing();
    bool isEditEnabled = (route != nullptr);
    bool isClearEnabled = editMode && (route != nullptr) && (route->buffer()->waypointsCount() > 0);

    bool canSave = false;

    if (editMode)
        canSave = (route != nullptr) && (route->buffer()->waypointsCount() > 0);
    else
        canSave = (route != nullptr) && (route->waypointsCount() > 0);
    _editRouteButton->setEnabled(route);
    _saveRouteButton->setEnabled(canSave);
    _loadRouteButton->setEnabled(route != nullptr);
    _copyRouteButton->setEnabled(editMode);
    _clearRouteButton->setEnabled(isClearEnabled);
    _uploadRouteButton->setEnabled(editMode);
}


bmcl::OptionPtr<mccuav::Route> RoutesListWidget::currentRoute() const
{
    auto uav = _uavController->selectedUav();
    assert(uav);
    if (!uav)
        return bmcl::None;
    QModelIndex currentIndex = _routesView->selectionModel()->currentIndex();
    return uav->findRoute(_model->data(_filterModel->mapToSource(currentIndex), RouteListModel::RouteId).toInt());
}

void RoutesListWidget::selectRoute(const QItemSelection &selected, const QItemSelection &deselected)
{
    if (selected.indexes().empty())
        return;
    QModelIndex index = selected.indexes()[0];
    auto uav = _uavController->selectedUav();
    if (!uav)
    {
        assert(false);
        return;
    }

    auto r = uav->findRouteById(index.data(RouteListModel::RouteId).toInt());
    if (!r)
    {
        assert(false);
        return;
    }
    _routesController->selectRoute(r);
}

void RoutesListWidget::routeDoubleClicked(const QModelIndex& index)
{
    if (!(index.flags() & Qt::ItemIsEnabled))
        return;

    if (index.column() == 0)
    {
        auto route = _routesController->selectedRoute();
        QColor newColor = QColorDialog::getColor(route->pen().color(), this, "Цвет маршрута", mccui::colorDialogOptions());

        if (!newColor.isValid())
            return;

        route->setColor(newColor);
    }
    else
    {
        auto uav = _uavController->selectedUav();
        assert(uav);
        auto routeId = _model->data(_filterModel->mapToSource(index), RouteListModel::RouteId).toInt();
        auto route = uav->findRouteById(routeId);
        if (route == nullptr)
            return;
        _routesController->centerOnRoute(route);
    }
}

void RoutesListWidget::contextMenuRequested(const QPoint& pos)
{
    if (!_uavController->selectedUav() || _model->rowCount() == 0)
        return;
    auto uav = _uavController->selectedUav();
    QMenu menu;
    auto currentIndex = _routesView->indexAt(pos);

    if (currentIndex.isValid()) // Кликнули по маршруту
    {
        auto route = uav->findRoute(_model->data(_filterModel->mapToSource(currentIndex), RouteListModel::RouteId).toInt());
        if (uav->activeRoute() != route)
            addMenuAction(&menu, "Сменить маршрут", [this, route]() { setRouteActive(route.unwrap()); });
        addMenuAction(&menu, "Нет активного маршрута", [this]() {setRouteEmpty(); });
        menu.addSeparator();
        auto changeDirectionText = QString("Изменить направление: %1").arg(route->isInverted() ? "вперед" : "назад");
        addMenuAction(&menu, changeDirectionText, [this, route]() {changeRouteDirection(route.unwrap()); });
        menu.addSeparator();
        addMenuAction(&menu, "С диска...", [this, route]() { loadFromDisk(route.unwrap()); });
        addMenuAction(&menu, "На диск...", [this, route]() { saveToDisk(route.unwrap()); });
        addMenuAction(&menu, "Экспорт маршрута в KML...", [this, route]() { exportToKml(route.unwrap()); });
        menu.addSeparator();
    }
    else 
    {
        if (uav->activeRoute() != nullptr)
        {
            addMenuAction(&menu, "Нет активного маршрута", [this]() { setRouteEmpty(); });
            menu.addSeparator();
        }
    }
    addMenuAction(&menu, "Экспорт всех маршрутов в KML...", [this]() { exportAllToKml(); });
    addMenuAction(&menu, "Все маршруты на диск...", [this]() { saveAllToDisk(); });

    menu.exec(_routesView->viewport()->mapToGlobal(pos));
}

void RoutesListWidget::copyRouteButtonClicked()
{
    auto uav = _uavController->selectedUav();
    assert(uav);
    if (uav == nullptr)
        return;
    auto route = currentRoute();
    if (route.isNone())
        return;
    uav->resetEditableRoute(route.take());
}

void RoutesListWidget::clearRouteButtonClicked()
{
    assert(_routesController->isEditing());
    auto uav = _uavController->selectedUav();
    assert(uav);
    if (uav == nullptr)
        return;
    auto route = currentRoute();
    if (route.isNone())
        return;
    route->clear();
}

void RoutesListWidget::uploadRouteButtonClicked()
{
    assert(_routesController->isEditing());
    auto uav = _uavController->selectedUav();
    assert(uav);
    if (uav == nullptr)
        return;
    auto route = currentRoute();
    if (route.isNone())
        return;

//     if (_selectedRoute->readOnly())
//     {
//         QMessageBox::warning(this, "Загрузка маршрута на борт", "Невозможно загрузить маршрут: только для чтения");
//         return;
//     }

    uav->uploadEditableRoute();
}

void RoutesListWidget::saveRouteButtonClicked()
{
    auto route = currentRoute();
    assert(route.isSome());
    if (route.isNone())
        return;
    saveToDisk(route.take());
}

void RoutesListWidget::loadRouteButtonClicked()
{
    auto route = currentRoute();
    assert(route.isSome());
    if (route.isNone())
        return;
    loadFromDisk(route.take());
}

void RoutesListWidget::setRouteActive(mccuav::Route* route)
{
    auto uav = _uavController->selectedUav();
    assert(uav);
    assert(route);
    if (QMessageBox::question(this, "Смена маршрута", QString("Сменить маршрут на: %1").arg(route->name())) != QMessageBox::Yes)
        return;
    uav->setActiveRoute(route);
}

void RoutesListWidget::setRouteEmpty()
{
    auto uav = _uavController->selectedUav();
    assert(uav);
    if (QMessageBox::question(this, "Смена маршрута", QString("Задать отсутствие активного маршрута?")) != QMessageBox::Yes)
        return;
    uav->setEmptyActiveRoute();
}

void RoutesListWidget::changeRouteDirection(mccuav::Route* route)
{
    auto uav = _uavController->selectedUav();
    assert(uav);
    assert(route);
    uav->changeRouteDirection(route, route->isInverted());
}

void RoutesListWidget::loadFromDisk(mccuav::Route* route)
{
    assert(route);

    auto routesDir = _routesDirWriter->read("").toString();

    QString fname = QFileDialog::getOpenFileName(this, "Открытие маршрута", routesDir, "Route files (*.route)");
    if (fname.isEmpty())
        return;
    _routesDirWriter->write(QFileInfo(fname).absolutePath());
    _routesController->startEditRoute();
    route->load(fname, _pController);
}

void RoutesListWidget::saveToDisk(mccuav::Route* route)
{
    assert(route);
    QString defaultName = route->generateSaveName() + ".route";
    auto routesDir = _routesDirWriter->read("").toString();
    QString fileName = QFileDialog::getSaveFileName(this, "Сохранить маршрут", routesDir + QDir::separator() + defaultName, "Маршруты (*.route)");
    if (fileName.isEmpty())
        return;
    _routesDirWriter->write(QFileInfo(fileName).absolutePath());
    route->save(fileName, mccuav::Route::FileFormat::Json);
}

void RoutesListWidget::exportToKml(mccuav::Route* route)
{
    assert(route);
    QString defaultName = route->generateSaveName() + ".kml";
    auto routesDir = _routesDirWriter->read("").toString();
    QString fileName = QFileDialog::getSaveFileName(this, "Экспортировать маршрут в KML", routesDir + QDir::separator() + defaultName, "KML (*.kml)");
    if (fileName.isEmpty())
        return;
    _routesDirWriter->write(QFileInfo(fileName).absolutePath());
    route->save(fileName, mccuav::Route::FileFormat::KmlLineString);
}

void RoutesListWidget::exportAllToKml()
{
    auto uav = _uavController->selectedUav();
    assert(uav);
    auto routesDir = _routesDirWriter->read("").toString();
    QString dirName = QFileDialog::getExistingDirectory(this, "Экспортировать все маршруты в KML", routesDir);
    if (dirName.isEmpty())
        return;

    _routesDirWriter->write(dirName);
    for (auto r : uav->routes())
    {
        QString defaultName = r->generateSaveName() + ".kml";
        r->save(dirName + "/" + defaultName, mccuav::Route::FileFormat::KmlLineString);
    }
}

void RoutesListWidget::saveAllToDisk()
{
    auto uav = _uavController->selectedUav();
    assert(uav);
    auto routesDir = _routesDirWriter->read("").toString();
    QString dirName = QFileDialog::getExistingDirectory(this, "Сохранить все маршруты", routesDir);
    if (dirName.isEmpty())
        return;

    for (auto r : uav->routes())
    {
        if (!r->isEnabled())
            return;
        QString defaultName = r->generateSaveName() + mccuav::Route::filenameExtension();
        r->save(dirName + "/" + defaultName, mccuav::Route::FileFormat::Json);
    }
}

EditedRouteHeaderWidget::EditedRouteHeaderWidget(QWidget* parent, mccuav::RoutesController* routesController) : QWidget(parent)
{
    auto layout = new QHBoxLayout();
    _color = new QLabel(this);
    _color->setFixedSize(20, 20);
    _info = new QLabel(this);
    QFont f;
    f.setBold(true);
    _info->setFont(f);
    layout->addWidget(_color);
    layout->addWidget(_info);
    layout->addStretch();
    setLayout(layout);
    layout->setContentsMargins(0, 0, 0, 0);
    setVisible(false);

    connect(routesController, &mccuav::RoutesController::routeEditingChanged, this,
            [this, routesController](bool isEditing)
            {
                setVisible(isEditing);
                if(isEditing)
                    setRoute(routesController->selectedRoute());
            }
    );
}

void EditedRouteHeaderWidget::setRoute(mccuav::Route* route)
{
    if (!route)
        return;

    _info->setText(route->name());
    _color->setStyleSheet(QString("background-color: %1;"
                          "border: 1px;"
                          "border-color: black;"
                          "border-style: outset;").arg(route->pen().color().name()));
}

RoutePropertiesWidget::RoutePropertiesWidget(QWidget* parent, mccuav::RoutesController* routesController)
    : QWidget(parent), _route(nullptr), _routesController(routesController)
{
    _loopCheckbox = new QCheckBox("Кольцевой", this);
    _revesedCheckbox = new QCheckBox("В обр. направлении", this);
    _showPointsOnlyCheckbox = new QCheckBox("Только точки", this);

    auto layout = new QHBoxLayout();
    setLayout(layout);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(_loopCheckbox);
    layout->addWidget(_revesedCheckbox);
    layout->addWidget(_showPointsOnlyCheckbox);
    layout->addStretch();

    connect(routesController, &mccuav::RoutesController::selectedRouteChanged, this, [this](mccuav::Route* r, mccuav::Uav*) { setRoute(r); });
    
    connect(routesController, &mccuav::RoutesController::routeEditingChanged, this,
            [this, routesController](bool isEditing)
            {
                if (isEditing)
                    setRoute(routesController->selectedRoute()->buffer());
                else
                    setRoute(routesController->selectedRoute());
            }
    );


    connect(_loopCheckbox, &QCheckBox::stateChanged,            this,
            [this](bool checked)
            {
                assert(_route);
                _route->setClosedPath(checked);
            }
    );
    connect(_revesedCheckbox, &QCheckBox::stateChanged,         this,
            [this](bool checked)
            {
                assert(_route);
                _route->setInverted(checked);
            }
    );
    connect(_showPointsOnlyCheckbox, &QCheckBox::stateChanged,  this,
            [this](bool checked)
            {
                assert(_route);
                _route->setShowPointsOnly(checked);
            }
    );
}

void RoutePropertiesWidget::setRoute(mccuav::Route* r)
{
    qDebug() << "RoutePropertiesWidget::setRoute " << r;
    if (_route)
    {
        disconnect(_route, &mccuav::Route::showPointsOnlyFlagChanged, this, &RoutePropertiesWidget::updateButtons);
        disconnect(_route, &mccuav::Route::inverseFlagChanged, this, &RoutePropertiesWidget::updateButtons);
        disconnect(_route, &mccuav::Route::closedPathFlagChanged, this, &RoutePropertiesWidget::updateButtons);
    }
    _route = r;
    if (_route)
    {
        connect(_route, &mccuav::Route::showPointsOnlyFlagChanged, this, &RoutePropertiesWidget::updateButtons);
        connect(_route, &mccuav::Route::inverseFlagChanged, this, &RoutePropertiesWidget::updateButtons);
        connect(_route, &mccuav::Route::closedPathFlagChanged, this, &RoutePropertiesWidget::updateButtons);
    }
    updateButtons();
}

void RoutePropertiesWidget::updateButtons()
{
    bool isEditing = _routesController->isEditing();
    _loopCheckbox->setEnabled(isEditing);
    _revesedCheckbox->setEnabled(isEditing);
    _showPointsOnlyCheckbox->setEnabled(isEditing);

    QObjectSignalBlocker b1(_loopCheckbox);
    QObjectSignalBlocker b2(_revesedCheckbox);
    QObjectSignalBlocker b3(_showPointsOnlyCheckbox);
    if (!_route)
    {
        _loopCheckbox->setChecked(false);
        _revesedCheckbox->setChecked(false);
        _showPointsOnlyCheckbox->setChecked(false);
        return;
    }
    _loopCheckbox->setChecked(_route->isLoop());
    _revesedCheckbox->setChecked(_route->isInverted());
    _showPointsOnlyCheckbox->setChecked(_route->showPointsOnly());
}

WaypointsEditorWidget::WaypointsEditorWidget(QWidget* parent, const mccmap::MapRect* rect, const mccui::CoordinateSystemController* csController, mccui::Settings* settings, mccuav::UavController* uavController, mccuav::RoutesController* routesController)
    : QWidget(parent)
    , _route(nullptr)
    , _mapRect(rect)
    , _uavController(uavController)
    , _routesController(routesController)
{
    _addButton = new QPushButton("+", this);
    _moveUpButton = new QPushButton("▲", this);
    _moveDownButton = new QPushButton("▼", this);
    _removeButton = new QPushButton("-", this);
    _inverseButton = new QPushButton("◄─►", this);
    _scanRectButton = new QPushButton("", this);
    _scanAreaButton = new QPushButton("", this);
    _scanRectButton->setIcon(mccres::loadPixmap(mccres::ResourceKind::RouteShapeRect));
    _scanAreaButton->setIcon(mccres::loadPixmap(mccres::ResourceKind::RouteShapeArea));

    _addButton->setToolTip("Добавить точку");
    _moveUpButton->setToolTip("Переместить вверх в очереди");
    _moveDownButton->setToolTip("Переместить вниз в очереди");
    _removeButton->setToolTip("Удалить точку");
    _inverseButton->setToolTip("Поменять направление маршрута на противоположное");
    _scanRectButton->setToolTip("Создать маршрут \"Прямоугольник\"");
    _scanAreaButton->setToolTip("Создать маршрут \"Змейка\"");

    _addButton->setFixedSize(24, 24);
    _moveUpButton->setFixedSize(24, 24);
    _moveDownButton->setFixedSize(24, 24);
    _removeButton->setFixedSize(24, 24);
    _inverseButton->setFixedSize(24, 24);
    _scanRectButton->setFixedSize(24, 24);
    _scanAreaButton->setFixedSize(24, 24);

    auto buttonsLayout = new QHBoxLayout();
    buttonsLayout->addWidget(_addButton);
    buttonsLayout->addWidget(_moveUpButton);
    buttonsLayout->addWidget(_moveDownButton);
    buttonsLayout->addWidget(_removeButton);
    buttonsLayout->addWidget(_inverseButton);
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(_scanRectButton);
    buttonsLayout->addWidget(_scanAreaButton);
    buttonsLayout->setContentsMargins(0, 0, 0, 0);

    _editButtons = new QFrame(this);
    _editButtons->setLayout(buttonsLayout);

    _waypointsTable = new TableViewWithFreezeSelection(this);
    _waypointsTable->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(_waypointsTable, &QWidget::customContextMenuRequested, this, &WaypointsEditorWidget::contextMenuRequested);

    auto layout = new QVBoxLayout();
    layout->addWidget(_editButtons);
    layout->addWidget(_waypointsTable);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    _model = new MissionPlanModel(csController);

    auto delegate = new MissionPlanDelegate(settings, _model);
    _waypointsTable->setModel(_model);
    _waypointsTable->setItemDelegate(delegate);
    _waypointsTable->setAlternatingRowColors(true);
    _waypointsTable->setSelectionBehavior(QAbstractItemView::SelectItems);
    _waypointsTable->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    _waypointsTable->setColumnWidth(0, 10);

    _scanDialog = new ScanAreaDialog(this);

    connect(_scanDialog, &ScanAreaDialog::accepted, this, &WaypointsEditorWidget::scanAccepted);
    connect(_scanDialog, &ScanAreaDialog::rejected, this, &WaypointsEditorWidget::scanRejected);


    connect(routesController, &mccuav::RoutesController::selectedRouteChanged, this, [this](mccuav::Route* r, mccuav::Uav*) { setRoute(r); });
    connect(routesController, &mccuav::RoutesController::routeEditingChanged, this,
            [this, routesController](bool isEditing)
            {
                if (isEditing)
                    setRoute(routesController->selectedRoute()->buffer());
                else
                    setRoute(routesController->selectedRoute());
            }
    );
    
    connect(_addButton,         &QPushButton::pressed, this, &WaypointsEditorWidget::addButtonPressed);
    connect(_moveUpButton,      &QPushButton::pressed, this, &WaypointsEditorWidget::moveUpButtonPressed);
    connect(_moveDownButton,    &QPushButton::pressed, this, &WaypointsEditorWidget::moveDownButtonPressed);
    connect(_removeButton,      &QPushButton::pressed, this, &WaypointsEditorWidget::removeButtonPressed);
    connect(_inverseButton,     &QPushButton::pressed, this, &WaypointsEditorWidget::inverseButtonPressed);
    connect(_scanRectButton,    &QPushButton::pressed, this, &WaypointsEditorWidget::scanRectPressed);
    connect(_scanAreaButton,    &QPushButton::pressed, this, &WaypointsEditorWidget::scanAreaPressed);

    connect(_waypointsTable->selectionModel(), &QItemSelectionModel::selectionChanged, this,
            [this]()
    {
        updateButtons();

        if (_route)
        {
            QModelIndexList selectedIndexes = _waypointsTable->selectionModel()->selectedIndexes();
            std::vector<size_t> indexes;
            for (const auto& i : selectedIndexes)
                indexes.emplace_back(i.row());

            _route->setSelectedPoints(indexes);
        }
    });

    connect(_waypointsTable, &QTableView::doubleClicked, this, &WaypointsEditorWidget::waypointDoubleClicked);

    setRoute(nullptr);
}

void WaypointsEditorWidget::setRoute(mccuav::Route* route)
{
    _route = route;
    _model->setRoute(route);
    setEnabled(route != nullptr);
    _waypointsTable->resizeColumnsToContents();
    updateButtons();
}

void WaypointsEditorWidget::contextMenuRequested(const QPoint& pos)
{
    if (!_uavController->selectedUav() || _model->rowCount() == 0)
        return;
    auto uav = _uavController->selectedUav();
    bool editMode = _routesController->isEditing();
    auto currentIndex = _waypointsTable->indexAt(pos);
    auto column = (MissionPlanModel::Columns)currentIndex.column();
    QMenu menu;
    if (!editMode && column == MissionPlanModel::Columns::Index)
    {
        addMenuAction(&menu, "Убрать активную точку", [this]() {clearActivePoint(); });
    }
    else if (editMode && column != MissionPlanModel::Columns::Index && column != MissionPlanModel::Columns::WaypointType)
    {
        addMenuAction(&menu, "Задать выделенные значения", [this]() { applyToSelected(); });
        addMenuAction(&menu, "Задать все значения", [this]() { applyToAll(); });
    }
    else
        return;

    menu.exec(_waypointsTable->viewport()->mapToGlobal(pos));
}

void WaypointsEditorWidget::waypointDoubleClicked(const QModelIndex& index)
{
    auto routeEditMode = MissionPlanModel::columnToEditMode((MissionPlanModel::Columns)index.column());

    bool editMode = _routesController->isEditing();

    if (editMode)
    {
        if (index.column() == 0)
        {
            assert(_route);
            _route->setActivePoint(index.row());
        }
        else
        {
            emit _route->showEditDialog(index.row(), routeEditMode);
        }
        return;
    }
    else
    {
        mccuav::Uav* uav = _uavController->selectedUav();
        assert(uav);
        if (index.column() > 0)
        {
            QModelIndex lastIndex = index;
            _routesController->startEditRoute();
            emit _route->showEditDialog(lastIndex.row(), routeEditMode);
        }
        else
        {
            int wpIdx = index.row();

            if (QMessageBox::question(this, "Смена активной точки", QString("Сменить активную точку маршрута %1 на: %2")
                .arg(_route->name())
                .arg(wpIdx + 1)) != QMessageBox::Yes)
                return;

            uav->setNextWaypoint(_route, wpIdx);
        }
    }
}

void WaypointsEditorWidget::addButtonPressed()
{
    assert(_uavController->selectedUav());
    assert(_route);
    mccgeo::LatLon latLon;
    const mccmsg::Waypoints& lst = _route->waypointsList();
    int count = _route->waypointsCount();
    int offset = 40;
    if (count == 0)
    {
        latLon = _mapRect->centerLatLon();
    }
    else if (count == 1)
    {
        latLon = _mapRect->latLonOnLine(lst.front().position, 0, offset);
    }
    else
    {
        double angle = _mapRect->angleBetween(lst[lst.size() - 2].position, lst.back().position);
        latLon = _mapRect->latLonOnLine(lst.back().position, angle, offset);
    }
    mccmsg::Waypoint wp;
    wp.position.latitude() = latLon.latitude();
    wp.position.longitude() = latLon.longitude();
    if (!lst.empty())
    {
        const auto& lastWp = lst.back();
        wp.position.altitude() = lastWp.position.altitude();
        wp.speed = lastWp.speed;
    }
    else
    {
        wp.position.altitude() = _uavController->calcWaypointAltitudeAt(latLon);
    }
    _route->addWaypoint(wp);
    _route->setSelectedPoint(_route->waypointsCount() - 1);
    _waypointsTable->scrollToBottom();
}

void WaypointsEditorWidget::moveUpButtonPressed()
{
    assert(_uavController->selectedUav());

    QItemSelectionModel* selection = _waypointsTable->selectionModel();

    if (!selection->hasSelection())
        return;

    int rowIdx = selection->selectedIndexes().first().row();

    int wpIdx = _model->data(_model->index(rowIdx, static_cast<int>(MissionPlanModel::Columns::Index))).toInt();

    _route->moveWaypointUp(wpIdx - 1);

    if (rowIdx < 1)
        rowIdx = 1;

    _waypointsTable->selectRow(rowIdx - 1);
}

void WaypointsEditorWidget::moveDownButtonPressed()
{
    assert(_uavController->selectedUav());

    QItemSelectionModel* selection = _waypointsTable->selectionModel();

    if (!selection->hasSelection())
        return;

    int rowIdx = selection->selectedIndexes().first().row();
    int wpIdx = _model->data(_model->index(rowIdx, static_cast<int>(MissionPlanModel::Columns::Index))).toInt();

    _route->moveWaypointDown(wpIdx - 1);

    if (rowIdx > _model->rowCount())
        rowIdx = _model->rowCount() - 1;

    _waypointsTable->selectRow(rowIdx + 1);
}

void WaypointsEditorWidget::removeButtonPressed()
{
    assert(_uavController->selectedUav());
    assert(_route);

    QItemSelectionModel* selection = _waypointsTable->selectionModel();

    if (!selection->hasSelection())
        return;

    int rowIdx = selection->selectedIndexes().first().row();

    int wpIdx = _model->data(_model->index(rowIdx, static_cast<int>(MissionPlanModel::Columns::Index))).toInt();

    _route->removeWaypoint(wpIdx - 1);

    if (rowIdx >= _model->rowCount())
        rowIdx = _model->rowCount() - 1;

    _waypointsTable->selectRow(rowIdx);
}

void WaypointsEditorWidget::inverseButtonPressed()
{
    assert(_uavController->selectedUav());
    assert(_route);
    _route->reverse();
}

void WaypointsEditorWidget::scanAreaPressed()
{
    emit _uavController->beginEditTemplate(mccuav::WaypointTempalteType::Scan);
    _scanDialog->setType(mccuav::WaypointTempalteType::Scan);
    _scanDialog->show();
}

void WaypointsEditorWidget::scanRectPressed()
{
    emit _uavController->beginEditTemplate(mccuav::WaypointTempalteType::Rectangle);
    _scanDialog->setType(mccuav::WaypointTempalteType::Rectangle);
    _scanDialog->show();
}

void WaypointsEditorWidget::scanAccepted()
{
    _scanDialog->close();
    emit _uavController->endEditTemplate(_scanDialog->delta(), _scanDialog->height(), _scanDialog->speed());
}

void WaypointsEditorWidget::scanRejected()
{
    _scanDialog->close();
    emit _uavController->resetEditTemplate(_scanDialog->type());
}

void WaypointsEditorWidget::clearActivePoint()
{
    auto uav = _uavController->selectedUav();
    assert(uav);
    assert(_route);
    if (_routesController->isEditing())
    {
        _route->setActivePoint(bmcl::None);
    }
    else
    {
        uav->setNextWaypoint(_route, -1);
    }
}

void WaypointsEditorWidget::applyToSelected()
{
    auto indices = _waypointsTable->selectionModel()->selectedIndexes();

    if (indices.empty())
        return;

    auto column = static_cast<MissionPlanModel::Columns>(indices.first().column());

    auto valueName = _model->headerData(indices.first().column(), Qt::Horizontal, Qt::DisplayRole).toString();

    bool ok = false;

    auto value = QInputDialog::getDouble(this, "Ввод нового значения:", QString("%1:").arg(valueName), indices.first().data().toDouble(), 0.0, std::numeric_limits<double>::max(), 1, &ok);

    for (const auto& index : indices)
    {
        _model->setData(index, value, _model->roleForColumn(column));
    }
}

void WaypointsEditorWidget::applyToAll()
{
    auto indices = _waypointsTable->selectionModel()->selectedIndexes();

    if (indices.empty())
        return;

    int sourceColumn = indices.first().column();

    auto column = static_cast<MissionPlanModel::Columns>(sourceColumn);

    auto valueName = _model->headerData(sourceColumn, Qt::Horizontal, Qt::DisplayRole).toString();

    bool ok = false;
    auto value = QInputDialog::getDouble(this, "Ввод нового значения:", QString("%1:").arg(valueName), indices.first().data().toDouble(), 0.0, std::numeric_limits<double>::max(), 1, &ok);

    indices.clear();
    for (int i = 0; i < _model->rowCount(); ++i)
        indices.push_back(_model->index(i, sourceColumn));

    for (const auto& index : indices)
    {
        _model->setData(index, value, _model->roleForColumn(column));
    }
}

void WaypointsEditorWidget::updateButtons()
{
    bool editMode = _routesController->isEditing();
    bool isClearEnabled = editMode && (_route != nullptr) && (_route->waypointsCount() > 0);
    bool isRemoveEnabled = editMode && !_waypointsTable->selectionModel()->selectedIndexes().isEmpty();

    _editButtons->setVisible(editMode);
    _removeButton->setEnabled(isRemoveEnabled);
    _moveUpButton->setEnabled(isRemoveEnabled);
    _moveDownButton->setEnabled(isRemoveEnabled);
    _inverseButton->setEnabled(isClearEnabled);
}

RouteEditorTool::RouteEditorTool(const mccui::CoordinateSystemController* csController,
                                 const mccmap::MapRect* rect,
                                 mccui::Settings* settings,
                                 mccuav::RoutesController* routesController,
                                 mccuav::UavController* uavController,
                                 mccmsg::ProtocolController* pController,
                                 QWidget* parent /*= nullptr*/)
    : _mapRect(rect)
    , _routesController(routesController)
    , _uavController(uavController)
    , _routesDirWriter(settings->acquireSharedWriter(routesDirKey).unwrap())
    , _filenameFilter(QString("Файлы маршрутов (*%1)").arg(mccuav::Route::filenameExtension()))
{
    setObjectName("Маршруты");
    setWindowTitle("Маршруты");
    setWindowIcon(QIcon(":/widget-routes/routes_icon.png"));

    _routesListWidget = new RoutesListWidget(this, settings, uavController, routesController, pController);
    _editedRouteHeaderWidget = new EditedRouteHeaderWidget(this, routesController);
    _routePropertiesWidget = new RoutePropertiesWidget(this, routesController);
    _waypointsEditorWidget = new WaypointsEditorWidget(this, rect, csController, settings, uavController, routesController);

    auto mainLayout = new QVBoxLayout();
    mainLayout->addWidget(_routesListWidget);
    mainLayout->addWidget(_editedRouteHeaderWidget);
    mainLayout->addWidget(_routePropertiesWidget);
    mainLayout->addWidget(_waypointsEditorWidget);

    mainLayout->setContentsMargins(3, 3, 3, 3);
    setLayout(mainLayout);

}

RouteEditorTool::~RouteEditorTool()
{

}
