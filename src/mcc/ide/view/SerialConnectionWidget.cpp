#include "mcc/ide/view/SerialConnectionWidget.h"

#include <QWidget>
#include <QtSerialPort/QSerialPortInfo>
#include <QPushButton>
#include <bmcl/MakeRc.h>

#include "mcc/ide/view/AbstractConnectionWidget.h"
#include "ui_SerialConnectionWidget.h"

namespace mccide {

SerialConnectionWidget::SerialConnectionWidget(QWidget* parent)
{
    Q_UNUSED(parent);
    _ui = new Ui::SerialConnectionWidget;
    _ui->setupUi(this);

    _ui->advancedCheckBox->setChecked(false);
    _ui->groupBox->setVisible(false);

    updatePorts();

    connect(_ui->updatePortsButton, &QPushButton::clicked, this, &SerialConnectionWidget::updatePorts);
}

SerialConnectionWidget::~SerialConnectionWidget()
{
    delete _ui;
}

mccmsg::ChannelDescription SerialConnectionWidget::fillNetDescription(const mccmsg::Channel& channel, const mccmsg::Protocol& protocol, const QString& info, bool log, int timeout, bool isDynamicTimeout, bool isReadOnly, const bmcl::Option<std::chrono::seconds>& reconnect) const
{
    int baudRate = _ui->serialSpeedComboBox->currentText().toInt();
    auto serial = bmcl::makeRc<const mccmsg::NetSerialParams>(_ui->comPortComboBox->currentText().toStdString(), baudRate);
    return bmcl::makeRc<const mccmsg::ChannelDescriptionObj>(channel, protocol, info.toStdString(), serial, log, std::chrono::milliseconds(timeout), isDynamicTimeout, isReadOnly, reconnect, bmcl::None);
}

void SerialConnectionWidget::setSettings(const mccmsg::ChannelDescription& dscr)
{
    auto p = bmcl::dynamic_pointer_cast<const mccmsg::NetSerialParams>(dscr->params());
    if (dscr->params()->transport() != mccmsg::NetTransport::Serial || p.isNull())
    {
        Q_ASSERT(false);
        return;
    }

    _ui->comPortComboBox->setCurrentIndex(_ui->comPortComboBox->findText(QString::fromStdString(p->portName())));
    _ui->serialSpeedComboBox->setCurrentIndex(_ui->serialSpeedComboBox->findText(QString::number(p->baudRate())));
}

QVector<SerialPortInfo> SerialConnectionWidget::getSerialPortInfo()
{
    QVector<SerialPortInfo> ps;

    QSerialPortInfo info;
    auto ports = info.availablePorts();
    for (const auto& i : ports)
    {
        SerialPortInfo s;
        //s.busy             = i.isBusy();
        s.manufacturer = i.manufacturer();
#ifdef _WIN32
        s.portName = i.portName();
#else
        s.portName = i.systemLocation();
#endif
        s.id = i.productIdentifier();
        //s.serialNumber     = i.serialNumber();
        s.sysLocation = i.systemLocation();
        s.vendorIdentifier = i.vendorIdentifier();
        ps.push_back(s);
    }

    return ps;
};

void SerialConnectionWidget::updatePorts()
{
    auto ports = getSerialPortInfo();

    std::sort(ports.begin(), ports.end(), [](const SerialPortInfo& p1, const SerialPortInfo& p2){ return p1.portName < p2.portName; });

    _ui->comPortComboBox->clear();
    for (auto port : ports)
    {
        _ui->comPortComboBox->addItem(port.portName);
    }
}
}
