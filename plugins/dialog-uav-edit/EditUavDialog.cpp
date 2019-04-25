#include "EditUavDialog.h"

#include "mcc/res/Resource.h"
#include "mcc/uav/UavController.h"
#include "mcc/ui/ColorDialogOptions.h"

#include <QAbstractButton>
#include <QColorDialog>
#include <QDialogButtonBox>
#include <QEvent>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QGroupBox>
#include <QComboBox>
#include <QSpinBox>

constexpr const char* colorTemplate = "border: 1px solid #000000;"
                                      "border-radius: 5px;"
                                      "background-color: #%1;";

EditUavDialog::EditUavDialog(mccuav::UavController* uavController, QWidget* parent)
    : mccui::Dialog(parent)
    , _uavController(uavController)
    , _deviceId(mccmsg::Device())
    , _name(new QLineEdit())
    , _colorSelector(new QLabel)
    , _uavLogging(new QCheckBox)
{
    setWindowTitle("Свойства аппарата");

    QVBoxLayout* mainLayout = new QVBoxLayout();
    setLayout(mainLayout);

    QGridLayout* gridLayout = new QGridLayout();
    mainLayout->addLayout(gridLayout);

    gridLayout->addWidget(new QLabel("Название:"), 0, 0);
    gridLayout->addWidget(_name, 0, 1);

    gridLayout->addWidget(new QLabel("Цвет:"), 1, 0);

    _colorSelector->installEventFilter(this);
    gridLayout->addWidget(_colorSelector, 1, 1);

    gridLayout->addWidget(new QLabel("Журналирование:"), 2, 0);
    gridLayout->addWidget(_uavLogging, 2, 1);

    QGroupBox* trackSettingsGroupBox = new QGroupBox("Трек аппарата", this);
    QGridLayout* l1 = new QGridLayout();
    trackSettingsGroupBox->setLayout(l1);
    _trackMode = new QComboBox(this);
    _trackMode->addItem("Весь", QVariant::fromValue(mccuav::TrackMode::All));
    _trackMode->addItem("Расстояние", QVariant::fromValue(mccuav::TrackMode::Distance));
    _trackMode->addItem("Время", QVariant::fromValue(mccuav::TrackMode::Time));
    _trackMode->addItem("Не отображать", QVariant::fromValue(mccuav::TrackMode::None));

    _trackValue = new QSpinBox(this);
    _trackValue->setMinimum(1);
    _trackValue->setMaximum(10000);
    l1->addWidget(new QLabel("Режим:", this), 0, 0);
    _trackValueLabel = new QLabel("Значение:", this);
    l1->addWidget(_trackValueLabel, 1, 0);
    l1->addWidget(_trackMode, 0, 1);
    l1->addWidget(_trackValue, 1, 1);
    mainLayout->addWidget(trackSettingsGroupBox);

    connect(_trackMode, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            [this]()
            {
                _trackSettings->setMode(_trackMode->currentData().value<mccuav::TrackMode>());
                updateTrackWidgets();
            }
    );

    connect(_trackValue, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
            [this](int value)
            {
                _trackSettings->setValue(_trackValue->value());
            }
    );
    mainLayout->addStretch();

    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::StandardButton::Ok | QDialogButtonBox::StandardButton::Cancel, this);
    buttons->button(QDialogButtonBox::StandardButton::Ok)->setIcon(mccres::loadIcon(mccres::ResourceKind::OkButtonIcon));
    buttons->button(QDialogButtonBox::StandardButton::Cancel)->setIcon(mccres::loadIcon(mccres::ResourceKind::CancelButtonIcon));
    mainLayout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::rejected, this, &EditUavDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, this, &EditUavDialog::accept);
}

EditUavDialog::~EditUavDialog() {}

void EditUavDialog::setUav(const mccmsg::Device& deviceId)
{
    if(_deviceId == deviceId)
        return;

    _deviceId = deviceId;

    auto uav = _uavController->uav(_deviceId);
    if(uav.isNone() || uav.unwrap() == nullptr)
    {
        _name->clear();
        _colorSelector->setStyleSheet(QString(colorTemplate).arg("transparent"));
        _uavColor = QColor();
        return;
    }
    _trackSettings = uav->trackSettings();
    updateTrackWidgets();
    _name->setText(uav->getName());
    _uavColor = uav->color();
    _colorSelector->setStyleSheet(QString(colorTemplate).arg(_uavColor.rgb(), 6, 16, QLatin1Char('0')));
    _uavLogging->setChecked(uav->deviceDescription()->log());
}

const mccmsg::Device&EditUavDialog::uav() const
{
    return _deviceId;
}

void EditUavDialog::setName(const QString& text)
{
    if(text != _name->text())
        _name->setText(text);
}

QString EditUavDialog::name() const
{
    return _name->text();
}

bool EditUavDialog::eventFilter(QObject* watched, QEvent* event)
{
    if(event->type() == QEvent::MouseButtonPress)
    {
        if(watched == _colorSelector)
        {
            QColor newColor = QColorDialog::getColor(_uavColor,
                                                     this,
                                                     "Цвет аппарата",
                                                     mccui::colorDialogOptions());
            if(newColor.isValid())
            {
                _colorSelector->setStyleSheet(QString(colorTemplate).arg(newColor.rgb(), 6, 16, QLatin1Char('0')));
                _uavColor = newColor;
            }

            event->accept();
            return true;
        }
    }

    return mccui::Dialog::eventFilter(watched, event);
}

void EditUavDialog::accept()
{
    mccui::Dialog::accept();

    auto uav = _uavController->uav(_deviceId);
    if(uav.isNone() || uav.unwrap() == nullptr)
        return;

    bmcl::Option<std::string> updatedName;
    bmcl::Option<bool> updatedLogging;

    QString newName = name().simplified();
    if (uav->getName() != newName && !newName.isEmpty())
        updatedName = newName.toStdString();
    if (_uavLogging->isChecked() != uav->deviceDescription()->log())
        updatedLogging = _uavLogging->isChecked();

    _uavController->requestUavUpdate(uav->device(), bmcl::None, updatedName, updatedLogging, bmcl::None);
    uav->setColor(_uavColor);
    uav->setTrackSettings(_trackSettings->mode(), _trackSettings->seconds(), _trackSettings->meters());
    uav->saveSettings();
}

void EditUavDialog::updateTrackWidgets()
{
    int idx = _trackMode->findData(QVariant::fromValue(_trackSettings->mode()));
    _trackMode->blockSignals(true);
    _trackMode->setCurrentIndex(idx);
    _trackMode->blockSignals(false);
    switch(_trackSettings->mode())
    {
    case mccuav::TrackMode::All:
    case mccuav::TrackMode::None:
        _trackValueLabel->setVisible(false);
        _trackValue->setVisible(false);
        break;
    case mccuav::TrackMode::Distance:
        _trackValueLabel->setVisible(true);
        _trackValue->setVisible(true);
        _trackValue->setSuffix(" м");
        break;
    case mccuav::TrackMode::Time:
        _trackValueLabel->setVisible(true);
        _trackValue->setVisible(true);
        _trackValue->setSuffix(" с");
        break;
    }

    if(_trackSettings.isSome())
    {
        _trackValue->setValue(_trackSettings->value());
    }
}
