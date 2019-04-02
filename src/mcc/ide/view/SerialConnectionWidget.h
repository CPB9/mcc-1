#pragma once

#include "mcc/Config.h"
#include "mcc/ide/view/AbstractConnectionWidget.h"

#include <QString>

namespace Ui { class SerialConnectionWidget; }

namespace mccide {

struct SerialPortInfo
{
    bool    busy;
    QString manufacturer;
    QString portName;
    int     id;
    //QString serialNumber;
    QString sysLocation;
    int     vendorIdentifier;
};

class MCC_IDE_DECLSPEC SerialConnectionWidget : public AbstractConnectionWidget
{
    Q_OBJECT
public:
    SerialConnectionWidget(QWidget* parent = 0);
    ~SerialConnectionWidget();

    mccmsg::ChannelDescription fillNetDescription(const mccmsg::Channel& channel, const mccmsg::Protocol& protocol, const QString& info, bool log, int timeout, bool isDynamicTimeout, bool isReadOnly, const bmcl::Option<std::chrono::seconds>& reconnect) const override;
    void setSettings(const mccmsg::ChannelDescription& dscr) override;
    static QVector<SerialPortInfo> getSerialPortInfo();

private slots:
    void updatePorts();

private:
    Ui::SerialConnectionWidget* _ui;
};
}
