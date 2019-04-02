#include "CoordinateConverterTool.h"
#include "CoordinateSystemSettingsPage.h"

#include "mcc/geo/Coordinate.h"
#include "mcc/geo/Position.h"
#include "mcc/uav/GlobalActions.h"
#include "mcc/ui/CoordinateEditor.h"
#include "mcc/ui/CoordinateSystem.h"
#include "mcc/ui/CoordinateSystemController.h"
#include "mcc/ui/Settings.h"
#include "mcc/res/Resource.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QToolButton>

#include <bmcl/Logging.h>

CoordinateConverterTool::~CoordinateConverterTool()
{}

CoordinateConverterTool::CoordinateConverterTool(const mccui::CoordinateSystemController* csController,
                                                 mccuav::GlobalActions* actions,
                                                 CoordinateSettingsPage* settings,
                                                 QWidget* parent)
    : mccui::Dialog(parent)
    , _csController(csController)
    , _actions(actions)
    , _settings(settings)
    , _firstPosition()
    , _secondPosition()
    , _mainLayout(new QGridLayout(this))
    , _orderButton(nullptr) // new QToolButton(this)),
    , _firstLatitude(new mccui::CoordinateEditor())
    , _firstLongitude(new mccui::CoordinateEditor())
    , _firstAltitude(new QDoubleSpinBox())
    , _latitudeLabel(new QLabel())
    , _longitudeLabel(new QLabel())
    , _altitudeLabel(new QLabel())
    , _secondLatitude(new mccui::CoordinateEditor())
    , _secondLongitude(new mccui::CoordinateEditor())
    , _secondAltitude(new QDoubleSpinBox())
    , _firstSystem(new QComboBox())
    , _secondSystem(new QComboBox())
    , _inputFormat(new QComboBox())
    , _pasteButton(new QToolButton())
    , _copyButton(new QToolButton())
    , _inCalculating(false)
    , _isLatitudeFirst(true)
{
    setModal(false);

    if(_settings->showConverterOnTopState())
        setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

    connect(_settings, &CoordinateSettingsPage::showConverterOnTopChanged, this,
            [this](bool show)
    {
        bool isVisible = this->isVisible();

        if(show)
            setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
        else
            setWindowFlags(windowFlags() &~ Qt::WindowStaysOnTopHint);

        setVisible(isVisible);
    });

    setObjectName("Конвертер координат");
    setWindowTitle("Конвертер координат");
    setWindowIcon(QIcon(":/toolbar-coordinatesystem/resources/converter_icon.png"));


    _actionCopyD = new QAction("Копировать (Г)", this);
    _actionCopyDm = new QAction("Копировать (ГМ)", this);
    _actionCopyDms = new QAction("Копировать (ГМС)", this);

//    _orderButton->setObjectName("_orderButton");
//    _orderButton->setGeometry(QRect(140, 28, 20, 10));
//    _orderButton->setToolTip("Поменять местами долготу/широту");
//    QFont font;
//    font.setPointSize(8);
//    _orderButton->setFont(font);
//    _orderButton->setText("▲▼");

    _firstLatitude->setObjectName("_firstLatitude");
    _firstLatitude->setMinMax(-89, 89);
    _firstLatitude->setToolTip("Редактировать первую широту");
    _mainLayout->addWidget(_firstLatitude, 0, 1);

    _firstLongitude->setObjectName("_firstLongitude");
    _firstLongitude->setMinMax(-180.0, 180.0);
    _firstLongitude->setToolTip("Редактировать первую долготу");
    _mainLayout->addWidget(_firstLongitude, 1, 1);

    _firstAltitude->setObjectName("_firstAltitude");
    _firstAltitude->setAlignment(Qt::AlignCenter);
    _firstAltitude->setSuffix("м");
    _firstAltitude->setDecimals(2);
    _firstAltitude->setMaximum(4e7);
    _firstAltitude->setMinimum(-6371e3);
    _firstAltitude->setToolTip("Редактировать первую высоту");
    _mainLayout->addWidget(_firstAltitude, 2, 1);

    _latitudeLabel->setObjectName("_latitudeLabel");
    _latitudeLabel->setAlignment(Qt::AlignCenter);
    _latitudeLabel->setText("Ш");
    _mainLayout->addWidget(_latitudeLabel, 0, 2);

    _longitudeLabel->setObjectName("_longitudeLabel");
    _longitudeLabel->setAlignment(Qt::AlignCenter);
    _longitudeLabel->setText("Д");
    _mainLayout->addWidget(_longitudeLabel, 1, 2);

    _altitudeLabel->setObjectName("_altitudeLabel");
    _altitudeLabel->setAlignment(Qt::AlignCenter);
    _altitudeLabel->setText("В");
    _mainLayout->addWidget(_altitudeLabel, 2, 2);

    _secondLatitude->setObjectName("_secondLatitude");
    _secondLatitude->setMinMax(-89, 89);
    _secondLatitude->setToolTip("Редактировать вторую широту");
    _mainLayout->addWidget(_secondLatitude, 0, 3);

    _secondLongitude->setObjectName("_secondLongitude");
    _secondLongitude->setMinMax(-180.0, 180.0);
    _secondLongitude->setToolTip("Редактировать вторую долготу");
    _mainLayout->addWidget(_secondLongitude, 1, 3);

    _secondAltitude->setObjectName("_secondAltitude");
    _secondAltitude->setAlignment(Qt::AlignCenter);
    _secondAltitude->setSuffix("м");
    _secondAltitude->setDecimals(2);
    _secondAltitude->setMaximum(4e7);
    _secondAltitude->setMinimum(-6371e3);
    _secondAltitude->setToolTip("Редактировать вторую высоту");
    _mainLayout->addWidget(_secondAltitude, 2, 3);

    QFrame *line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    _mainLayout->addWidget(line, 3, 1, 1, 3);


    QHBoxLayout *fLayout = new QHBoxLayout;
    fLayout->setSpacing(3);

    QIcon pasteIcon = mccres::loadIcon(mccres::ResourceKind::PasteIcon);

    _pasteButton->setObjectName("_pasteButton");
    _pasteButton->setToolTip("Вставить");
    _pasteButton->setIcon(pasteIcon);
    fLayout->addWidget(_pasteButton);

    _firstSystem->setObjectName("_firstFormat");
    _firstSystem->setToolTip("Выбор первой системы координат");
    setFormats(_firstSystem);
    fLayout->addWidget(_firstSystem, 1);

    _mainLayout->addLayout(fLayout, 4, 1);


    QHBoxLayout *cLayout = new QHBoxLayout;
    cLayout->setSpacing(3);

    _inputFormat->setObjectName("_inputFormat");
    _inputFormat->addItem("Г", static_cast<int>(mccui::AngularFormat::Degrees));
    _inputFormat->addItem("ГМ", static_cast<int>(mccui::AngularFormat::DegreesMinutes));
    _inputFormat->addItem("ГМС", static_cast<int>(mccui::AngularFormat::DegreesMinutesSeconds));
    _inputFormat->setToolTip("Выбор формата ввода и отображения координат");
    cLayout->addWidget(_inputFormat);

    _mainLayout->addLayout(cLayout, 4, 2);


    QHBoxLayout *sLayout = new QHBoxLayout;
    sLayout->setSpacing(3);

    _secondSystem->setObjectName("_secondFormat");
    _secondSystem->setToolTip("Выбор второй системы координат");
    setFormats(_secondSystem);
    sLayout->addWidget(_secondSystem);

    _copyButton->setObjectName("_copyButton");
    _copyButton->setToolTip("Копировать");

    QIcon copyIcon = mccres::loadIcon(mccres::ResourceKind::CopyIcon);

    _actionCopyD->setData(static_cast<int>(mccui::AngularFormat::Degrees));
    _actionCopyD->setIcon(copyIcon);

    _actionCopyDm->setData(static_cast<int>(mccui::AngularFormat::DegreesMinutes));
    _actionCopyDm->setIcon(copyIcon);

    _actionCopyDms->setData(static_cast<int>(mccui::AngularFormat::DegreesMinutesSeconds));
    _actionCopyDms->setIcon(copyIcon);

    QMenu *copyMenu = new QMenu(this);
    copyMenu->addAction(_actionCopyD);
    copyMenu->addAction(_actionCopyDm);
    copyMenu->addAction(_actionCopyDms);

    _copyButton->setMenu(copyMenu);
    _copyButton->setDefaultAction(_actionCopyD);

    sLayout->addWidget(_copyButton);

    _mainLayout->addLayout(sLayout, 4, 3);


    _mainLayout->setRowStretch(5, 1);
    _mainLayout->setColumnStretch(0, 1);
    _mainLayout->setColumnStretch(4, 1);
    _mainLayout->setSizeConstraint(QLayout::SetFixedSize);

    connect(_csController.get(), &mccui::CoordinateSystemController::changed,
            this, &CoordinateConverterTool::updateSettings);

    // From first to second
    connect(_firstLatitude,  &mccui::CoordinateEditor::valueChanged, this, &CoordinateConverterTool::handleFirstValues);
    connect(_firstLongitude, &mccui::CoordinateEditor::valueChanged, this, &CoordinateConverterTool::handleFirstValues);
    connect(_firstAltitude,  static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &CoordinateConverterTool::handleFirstValues);


    connect(_firstSystem, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentIndexChanged),
            this, &CoordinateConverterTool::handleFirstValues);
    connect(_secondSystem, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentIndexChanged),
            this, &CoordinateConverterTool::handleFirstValues);
    connect(_inputFormat, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentIndexChanged),
            this, &CoordinateConverterTool::updateUserFormat);

    // From second to first
    connect(_secondLatitude,  &mccui::CoordinateEditor::valueChanged, this, &CoordinateConverterTool::handleSecondValues);
    connect(_secondLongitude, &mccui::CoordinateEditor::valueChanged, this, &CoordinateConverterTool::handleSecondValues);
    connect(_secondAltitude,  static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &CoordinateConverterTool::handleSecondValues);

//    connect(_orderButton, &QToolButton::clicked, this, &CoordinateConverterTool::changeOrder);

    // Copy actions
    connect(_actionCopyD, &QAction::triggered, this, &CoordinateConverterTool::copyToClipboard);
    connect(_actionCopyDm, &QAction::triggered, this, &CoordinateConverterTool::copyToClipboard);
    connect(_actionCopyDms, &QAction::triggered, this, &CoordinateConverterTool::copyToClipboard);

    connect(_pasteButton, &QToolButton::clicked, this, &CoordinateConverterTool::pasteFromClipboard);


    updateSettings();
    recalculateFirstCoordinates();

    setAltitudeVisible(true);
    setShortNames(false);

    connect(_actions->showCoordinateConverterDialogAction(), &QAction::triggered, this,
            [this]()
    {
        show();
        activateWindow();
    });
}

void CoordinateConverterTool::copyToClipboard()
{
    // It's more useful format for conversations
    mccui::AngularFormat coordFormat = mccui::AngularFormat::Degrees;

    QAction *act = qobject_cast<QAction *>(sender());
    if(act)
    {
        coordFormat = static_cast<mccui::AngularFormat>(act->data().toInt());
    }

    //QMimeData* mData = _csController->makeShortMimeData(mccgeo::LatLon(_secondLatitude->value(), _secondLongitude->value()),
    //                                               static_cast<mccui::CoordinateSystem>(_secondSystem->currentData().toInt()),
    //                                               coordFormat);
    //
    //QApplication::clipboard()->setMimeData(mData);
}

void CoordinateConverterTool::pasteFromClipboard()
{
    //HACK
    bmcl::Option<mccgeo::LatLon> latLonOption = bmcl::None;//_csController->getLatLonFromClipboard();

    if(latLonOption.isSome())
    {
        _firstPosition.setLatLon(latLonOption.unwrap());
        _firstLatitude->setValue(_firstPosition.latitude());
        _firstLongitude->setValue(_firstPosition.longitude());

        recalculateFirstCoordinates();
    }
    else // Try to read text data
    {
        // It's enough 32 symbols. But we're getting it with a small reserve
        QString inputText = QApplication::clipboard()->text().left(256);
        if(!inputText.isEmpty())
        {
            inputText = inputText.simplified().replace("′", "'").replace("″", "\"");
            QStringList values = inputText.split(QRegularExpression(";|\\,|\\s+"));

            for(int i = 0; i<values.size();++i)
            {
                values[i].remove(QRegularExpression("[^0-9-+.'\"°sSwW]")); // only for minus sign

                QRegularExpression re("([sSwW]+$)|(^[sSwW]+)");
                if(re.match(values[i]).hasMatch())
                {
                    values[i] = "-" + values[i];
                }
                values[i].remove(QRegularExpression("[sSwW]"));
            }

                        values.removeAll(QString()); // Remove all empty lines

            if(values.size() > 0)
            {
                bmcl::Option<double> val = parseCoordinate(values[0]);
                if(val.isSome())
                {
                    if(_isLatitudeFirst)
                        _firstLatitude->setValue(val.unwrap());
                    else
                        _firstLongitude->setValue(val.unwrap());
                }
            }
            if(values.size() > 1)
            {
                bmcl::Option<double> val = parseCoordinate(values[1]);
                if(val.isSome())
                {
                    if(_isLatitudeFirst)
                        _firstLongitude->setValue(val.unwrap());
                    else
                        _firstLatitude->setValue(val.unwrap());
                }
            }
            if(values.size() > 2 && _firstAltitude->isEnabled())
            {
                bool ok(true);
                double val = values[2].toDouble(&ok);
                if(ok)
                    _firstAltitude->setValue(val);
            }

            recalculateFirstCoordinates();
        }
    }
}

void CoordinateConverterTool::updateSettings()
{
    // Setting coordinate system (current system is using at second ComboBox)
    int secIndex = _secondSystem->findData(static_cast<int>(_csController->currentSystemIndex()));
    _secondSystem->setCurrentIndex(secIndex);

    int firIndex(0);
    if(firIndex == secIndex && _firstSystem->count() > 1)
    {
        ++firIndex;
    }
    // Change input system only for clear data
    if(_firstLatitude->value() == 0.0 && _firstLongitude->value() == 0.0 && _firstAltitude->value() == 0.0)
    {
            _firstSystem->setCurrentIndex(firIndex);
    }

    // Other values will be set by updateUserFormat() slot
    _inputFormat->setCurrentIndex(_inputFormat->findData(static_cast<int>(_csController->angularFormat())));
}

void CoordinateConverterTool::setAltitudeVisible(bool visible)
{
    _firstAltitude->setVisible(visible);
    _altitudeLabel->setVisible(visible);
    _secondAltitude->setVisible(visible);
}

void CoordinateConverterTool::setShortNames(bool shortNames)
{
    if(shortNames)
    {
        _latitudeLabel->setText("Ш");
        _longitudeLabel->setText("Д");
        _altitudeLabel->setText("В");
    }
    else
    {
        _latitudeLabel->setText("Широта");
        _longitudeLabel->setText("Долгота");
        _altitudeLabel->setText("Высота");
    }
}

void CoordinateConverterTool::handleFirstValues()
{
    recalculateCoordinates();
}

void CoordinateConverterTool::handleSecondValues()
{
    recalculateCoordinates(false);
}

void CoordinateConverterTool::changeOrder()
{
    _isLatitudeFirst = !_isLatitudeFirst;

    int latRow(0), lonRow(1);
    if(!_isLatitudeFirst)
    {
        latRow = 1;
        lonRow = 0;
    }

    _mainLayout->takeAt(_mainLayout->indexOf(_firstLatitude));
    _mainLayout->takeAt(_mainLayout->indexOf(_firstLongitude));
    _mainLayout->takeAt(_mainLayout->indexOf(_secondLatitude));
    _mainLayout->takeAt(_mainLayout->indexOf(_secondLongitude));

    _mainLayout->takeAt(_mainLayout->indexOf(_latitudeLabel));
    _mainLayout->takeAt(_mainLayout->indexOf(_longitudeLabel));

    _mainLayout->addWidget(_firstLatitude,   latRow, 1);
    _mainLayout->addWidget(_firstLongitude,  lonRow, 1);
    _mainLayout->addWidget(_secondLatitude,  latRow, 3);
    _mainLayout->addWidget(_secondLongitude, lonRow, 3);

    _mainLayout->addWidget(_latitudeLabel,   latRow, 2);
    _mainLayout->addWidget(_longitudeLabel, lonRow, 2);
}

void CoordinateConverterTool::updateUserFormat()
{
    mccui::AngularFormat format = static_cast<mccui::AngularFormat>(_inputFormat->currentData().toInt());

    switch (format)
    {
        case mccui::AngularFormat::Degrees:
            _copyButton->setDefaultAction(_actionCopyD);
            break;
        case mccui::AngularFormat::DegreesMinutes:
            _copyButton->setDefaultAction(_actionCopyDm);
            break;
        case mccui::AngularFormat::DegreesMinutesSeconds:
            _copyButton->setDefaultAction(_actionCopyDms);
            break;
    }

    mccui::CoordinateFormat fmt(format);

    _firstLatitude->setCoordinateFormat(fmt);
    _firstLongitude->setCoordinateFormat(fmt);

    _secondLatitude->setCoordinateFormat(fmt);
    _secondLongitude->setCoordinateFormat(fmt);
}

void CoordinateConverterTool::setFormats(QComboBox *comboBox)
{
    if(comboBox) {
        for (std::size_t i = 0; i < _csController->systems().size(); i++) {
            const auto& desc = _csController->systems()[i];
            comboBox->addItem(desc.fullName(), static_cast<int>(i));
        }
    }
}

void CoordinateConverterTool::recalculateCoordinates(bool firstToSecond)
{
    if(!_inCalculating)
    {
        _inCalculating = true;

        if(firstToSecond)
        {
            recalculateFirstCoordinates();
        }
        else
        {
            recalculateSecondCoordinates();
        }

        _inCalculating = false;
    }
}

void CoordinateConverterTool::recalculateFirstCoordinates()
{
    if(_firstSystem->currentData() == _secondSystem->currentData())
    {
        _secondLatitude->setValue(_firstLatitude->value());
        _secondLongitude->setValue(_firstLongitude->value());
        _secondAltitude->setValue(_firstAltitude->value());

        return;
    }

    _firstPosition = mccgeo::Position(_firstLatitude->value(),
                                      _firstLongitude->value(),
                                      _firstAltitude->value());

    _secondPosition = convertPosition(_firstPosition,
                                      _firstSystem->currentData().toInt(),
                                      _secondSystem->currentData().toInt());

    _secondLatitude->setValue(_secondPosition.latitude());
    _secondLongitude->setValue(_secondPosition.longitude());
    _secondAltitude->setValue(_secondPosition.altitude());
}

void CoordinateConverterTool::recalculateSecondCoordinates()
{
    if(_firstSystem->currentData() == _secondSystem->currentData())
    {
        _firstLatitude->setValue(_secondLatitude->value());
        _firstLongitude->setValue(_secondLongitude->value());
        _firstAltitude->setValue(_secondAltitude->value());

        return;
    }

    _secondPosition = mccgeo::Position(_secondLatitude->value(),
                                       _secondLongitude->value(),
                                       _secondAltitude->value());

    _firstPosition = convertPosition(_secondPosition,
                                     _secondSystem->currentData().toInt(),
                                     _firstSystem->currentData().toInt());

    _firstLatitude->setValue(_firstPosition.latitude());
    _firstLongitude->setValue(_firstPosition.longitude());
    _firstAltitude->setValue(_firstPosition.altitude());
}

mccgeo::Position CoordinateConverterTool::convertPosition(const mccgeo::Position& fromPosition, int fromSystem, int toSystem)
{
    if(fromPosition.isValid()) {
        const mccgeo::CoordinateConverter* fromConv = _csController->systems()[fromSystem].converter();
        const mccgeo::CoordinateConverter* toConv = _csController->systems()[toSystem].converter();
        mccgeo::Coordinate coord = fromConv->convertInverse(fromPosition);
        return toConv->convertForward(coord).position();
    } else {
        BMCL_WARNING() << "Coordinates are not valid!";
    }

    return fromPosition;
}

constexpr QChar degreeChar(0260);

static bmcl::Option<double> parseDegrees(const QString& line)
{
    bool isOk;
    double res = line.split(degreeChar)[0].toDouble(&isOk);
    if (isOk) {
        return res;
    }
    return bmcl::None;
}

static bmcl::Option<double> parseDegreesMinutes(const QString& line)
{
    bool isOk;
    QStringList list = line.split(degreeChar);
    if (list.size() != 2) {
        return bmcl::None;
    }
    int deg = list[0].toInt(&isOk);
    if (!isOk) {
        return bmcl::None;
    }
    const QString& minStr = list[1];
    if (minStr.size() < 2 || minStr[minStr.size() - 1] != '\'') {
        return bmcl::None;
    }
    double min = minStr.left(minStr.size() - 1).toDouble(&isOk);
    if (!isOk || min < 0) {
        return bmcl::None;
    }
    return deg + min / 60.;
}

static bmcl::Option<double> parseDegreesMinutesSeconds(const QString& line)
{
    bool isOk;
    QStringList list = line.split(degreeChar);
    if (list.size() != 2) {
        return bmcl::None;
    }
    int deg = list[0].toInt(&isOk);
    if (!isOk) {
        return bmcl::None;
    }
    const QString& minSecStr = list[1];
    QStringList minSecList = minSecStr.split('\'');
    if (minSecList.size() != 2) {
        return bmcl::None;
    }
    const QString& minStr = minSecList[0];
    int min = minStr.toUInt(&isOk);
    if (!isOk || min < 0) {
        return bmcl::None;
    }
    const QString& secStr = minSecList[1];
    if (secStr.size() < 2 || secStr[secStr.size() - 1] != '"') {
        return bmcl::None;
    }
    double sec = secStr.left(secStr.size() - 1).toDouble(&isOk);
    if (!isOk) {
        return bmcl::None;
    }
    int sign(deg >= 0 ? 1 : -1);
    return deg + sign * min / 60. + sign * sec / 3600.;
}


bmcl::Option<double> CoordinateConverterTool::parseCoordinate(const QString &coordinate)
{
    if(coordinate.contains("°"))
    {
        if(coordinate.contains('"'))
        {
            return parseDegreesMinutesSeconds(coordinate);
        }
        else if(coordinate.contains('\''))
        {
            return parseDegreesMinutes(coordinate);
        }

        return parseDegrees(coordinate);
    }
    else
    {
        bool ok(true);
        double value = coordinate.toDouble(&ok);
        if(ok)
        {
            return value;
        }
    }

    return bmcl::None;
}
