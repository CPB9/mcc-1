#pragma once

#include "mcc/Config.h"
#include "mcc/ide/view/AbstractConnectionWidget.h"

class QValidator;

namespace Ui { class UdpConnectionWidget; }

namespace mccide {

class MCC_IDE_DECLSPEC UdpConnectionWidget : public AbstractConnectionWidget
{
public:
    UdpConnectionWidget(mccmsg::NetTransport transport, QWidget* parent = 0);
    ~UdpConnectionWidget();

    mccmsg::ChannelDescription fillNetDescription(const mccmsg::Channel& channel, const mccmsg::Protocol& protocol, const QString& info, bool log, int timeout, bool isDynamicTimeout, bool isReadOnly, const bmcl::Option<std::chrono::seconds>& reconnect) const override;
    void setSettings(const mccmsg::ChannelDescription& dscr) override;
private:
    mccmsg::NetUdpPtr getUdpParams() const;

    Ui::UdpConnectionWidget* _ui;

    QValidator* _sendValidator;
    QValidator* _listenValidator;
    mccmsg::NetTransport _transport;
};

}
