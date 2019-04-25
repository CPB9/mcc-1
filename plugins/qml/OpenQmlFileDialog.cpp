#include "OpenQmlFileDialog.h"

#include "mcc/uav/Uav.h"
#include "mcc/uav/UavController.h"

#include "mcc/ui/WidgetUtils.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QFileDialog>
#include <QPushButton>
#include <QToolButton>
#include <QFileInfo>

Q_DECLARE_METATYPE(mccmsg::Device);

OpenQmlFileDialog::OpenQmlFileDialog(const mccui::Rc<mccuav::UavController>& uavController, const QString& startPath, QWidget* parent)
    : QDialog(parent), _uavsController(uavController)
{
    setWindowTitle("Загрузка пользовательского интерфейса");

    _uavsCombo = new QComboBox();
    for (const auto& uav : uavController->uavsList())
    {
        QString deviceText = QString("%1 [%2]")
            .arg(uav->getInfo())
            .arg(uav->deviceId());

        _uavsCombo->addItem(deviceText, QVariant::fromValue(uav));
    }

    _path = new QLineEdit();
    auto openButton = new QToolButton();
    openButton->setText("...");

    auto deviceLayout = new QHBoxLayout();
    deviceLayout->addWidget(new QLabel("Аппарат:"));
    deviceLayout->addWidget(_uavsCombo);

    auto pathLayout = new QHBoxLayout();
    pathLayout->addWidget(new QLabel("Файл:"));
    pathLayout->addWidget(_path);
    pathLayout->addWidget(openButton);

    auto btnLayout = new QHBoxLayout();
    auto okBtn = new QPushButton("Открыть");
    okBtn->setEnabled(false);
    auto cancelBtn = new QPushButton("Отмена");
    btnLayout->addStretch();
    btnLayout->addWidget(okBtn);
    btnLayout->addWidget(cancelBtn);

    auto mainLayout = new QVBoxLayout();
    mainLayout->addLayout(deviceLayout);
    mainLayout->addLayout(pathLayout);
    mainLayout->addLayout(btnLayout);

    setLayout(mainLayout);

    connect(okBtn, &QPushButton::pressed, this, &QDialog::accept);
    connect(cancelBtn, &QPushButton::pressed, this, &QDialog::reject);
    connect(openButton, &QToolButton::pressed, this,
            [this, startPath]()
            {
                _path->setText(QFileDialog::getOpenFileName(this, "Open custom tool", startPath, "Qml (*.qml)"));
            }
    );

    connect(_path, &QLineEdit::textChanged, this, [this, okBtn]() { okBtn->setEnabled(!_path->text().isEmpty()); });
}

QString OpenQmlFileDialog::path() const
{
    return _path->text();
}

QString OpenQmlFileDialog::directory() const
{
    return QFileInfo(_path->text()).absoluteDir().absolutePath();
}

bmcl::OptionPtr<mccuav::Uav> OpenQmlFileDialog::uav() const
{
    auto deviceUuid = _uavsCombo->currentData();
    if (deviceUuid.isNull())
        return bmcl::None;
    return deviceUuid.value<mccuav::Uav*>();
}
