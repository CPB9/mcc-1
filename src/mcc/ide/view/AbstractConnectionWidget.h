#pragma once

#include <QWidget>

#include "mcc/Config.h"
#include "mcc/msg/obj/Channel.h"

namespace mccide {

class MCC_IDE_DECLSPEC AbstractConnectionWidget : public QWidget
{
    Q_OBJECT
public:
    inline AbstractConnectionWidget(QWidget* parent = 0)
        : QWidget(parent)
    {
    }

    virtual void setSettings(const mccmsg::ChannelDescription& dscr) {}
    virtual mccmsg::ChannelDescription fillNetDescription(const mccmsg::Channel& channel, const mccmsg::Protocol& protocol, const QString& info, bool log, int timeout, bool isDynamicTimeout, bool isReadOnly, const bmcl::Option<std::chrono::seconds>& reconnect) const = 0;
};
}
