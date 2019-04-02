#pragma once
#include "mcc/msg/Objects.h"
#include "mcc/msg/obj/Channel.h"
#include "mcc/uav/Fwd.h"
#include "mcc/uav/Rc.h"
#include "mcc/ui/Dialog.h"

#include <bmcl/Option.h>

#include <map>

class QLineEdit;
class QCheckBox;
class QSpinBox;
class QComboBox;

namespace mccide {
class AbstractConnectionWidget;
class UdpConnectionWidget;
class SerialConnectionWidget;

class MCC_IDE_DECLSPEC AddChannelDialog : public mccui::Dialog
{
    Q_OBJECT
public:
    AddChannelDialog(mccuav::ChannelsController* channelsController,
                     QWidget* parent = nullptr);
    ~AddChannelDialog() override;

    mccmsg::ChannelDescription netAddress(const mccmsg::Channel& channel = mccmsg::Channel()) const;
    mccmsg::Protocol protocol() const;

    const bmcl::Option<mccmsg::Channel>& editingChannel() const {return _channel;}

    void setProtocols(const mccmsg::ProtocolDescriptions& protocols);

    void setChannelDescription(const mccmsg::ChannelDescription& description);
    void setDefaultName(const QString& name);
    void setChannelName(const QString& name);
public slots:
    void setConnectionWidget(const QString& name);
    void show(const mccmsg::Channel& channel);

private:
    QLineEdit*  _channelName;
    QComboBox*  _channelType;
    QComboBox*  _protocol;
    QCheckBox*  _logExchange;
    QSpinBox*   _timeout;
    QCheckBox*  _dynamicTimeout;
    QCheckBox*  _readOnly;
    QString     _defaultName;
    std::map<QString, AbstractConnectionWidget*> _widgets;

    bmcl::Option<mccmsg::Channel> _channel;

    mccuav::Rc<mccuav::ChannelsController>  _channelsController;
};
}
