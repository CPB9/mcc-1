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

    _uavController->requestUavUpdate(uav->device(), bmcl::None, updatedName, updatedLogging);
    uav->setColor(_uavColor);
}
