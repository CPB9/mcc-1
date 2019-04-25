#pragma once
#include "mcc/Config.h"
#include <QObject>

#include <vector>
#include <map>
#include <set>
#include <bmcl/OptionRc.h>

#include "mcc/msg/obj/Channel.h"
#include "mcc/msg/Stats.h"
#include "mcc/plugin/PluginData.h"
#include "mcc/uav/Rc.h"
#include "mcc/ui/QObjectRefCountable.h"
#include "mcc/ui/Fwd.h"

class QWidget;

namespace mccuav {

class ExchangeService;

class MCC_UAV_DECLSPEC ChannelInformation
{
public:
//     explicit ChannelInformation(const mccmsg::Channel& channel)
//         : _channel(channel), _isActive(false) {}
    ChannelInformation(const mccmsg::Channel& channel, const mccmsg::ChannelDescription& channelDescription)
        : _isActive(false), _channel(channel), _channelDescription(channelDescription) {}

    const mccmsg::Channel& channel() const { return _channel; }

    bool isActive() const { return _isActive; }
    void setActive(bool isActive) { _isActive = isActive; }

    const bmcl::OptionRc<const mccmsg::ChannelDescriptionObj>& channelDescription() const { return _channelDescription; }
    void setChannelDescription(const mccmsg::ChannelDescription& channelDescription) { _channelDescription = channelDescription;}

    const bmcl::Option<mccmsg::StatChannel>& stat() const { return _stat; }
    void setStat(const mccmsg::StatChannel& stat) { _stat = stat; }

private:
    bool                                       _isActive;
    mccmsg::Channel                            _channel;
    bmcl::OptionRc<const mccmsg::ChannelDescriptionObj> _channelDescription;
    bmcl::Option<mccmsg::StatChannel>          _stat;
};
using ChannelInformations = std::vector<ChannelInformation>;

using UnknownUavs = std::map<mccmsg::Channel, std::set<mccmsg::DeviceId>>;

class MCC_UAV_DECLSPEC ChannelsController : public mccui::QObjectRefCountable<QObject>
{
    Q_OBJECT

public:
    explicit ChannelsController(mccui::Settings* settings, mccuav::ExchangeService* exchangeService, QObject* parent = nullptr);
    ~ChannelsController();

    bmcl::Option<const ChannelInformation&> channelInformation(const mccmsg::Channel& channel) const;
    bmcl::Option<const mccmsg::ProtocolDescription&> protocolDescription(const mccmsg::Protocol& protocol) const;

    const ChannelInformations& channelInformations() const;
    const mccmsg::ProtocolDescriptions& protocols() const;
    bmcl::Option<bool> isChannelActive(const mccmsg::Channel& channel) const;
    bmcl::Option<bool> isChannelReadOnly(const mccmsg::Channel& channel) const;
    bool isUavInChannel(const mccmsg::Device& device);
    const UnknownUavs& uavUnknowns() const;

    void requestChannelUpdate(const mccmsg::ChannelDescription&, QWidget* parent);
    void requestChannelActivate(const mccmsg::Channel& channel, bool state, QWidget* parent);
    void requestChannelUnregister(const mccmsg::Channel&, QWidget* parent);
    void requestChannelRegister(const mccmsg::ChannelDescription&, QWidget* parent);
    void setAllChannelsEnabled(bool isEnabled);

    size_t channelsCount() const {return _channels.size();}

signals:
    void protocolsChanged();
    void channelsChanged();
    void channelStatsUpdated(const mccmsg::Channel& channel, const mccmsg::StatChannel& stat);
    void channelActiveChanged(const mccmsg::Channel& channel, bool activated);
    void channelDescription(const mccmsg::Channel& channel, const mccmsg::ChannelDescription& description);
    void channelUnRegistered(const mccmsg::Channel& channel);

    void uavConnected(const mccmsg::Channel& channel, const mccmsg::Device& device);
    void uavDisconnected(const mccmsg::Channel& channel, const mccmsg::Device& device, const QString& error);
    void uavUnknownDetected();

private slots:
    void connectSlots();

    void onProtocolRegistered(const mccmsg::Protocol& protocol);

    void onChannelActivated(const mccmsg::Channel& channel, bool isActive);
    void onChannelUpdated(const mccmsg::ChannelDescription&);
    void onChannelRegistered(const mccmsg::Channel& channel);
    void onChannelUnRegistered(const mccmsg::Channel& channel);
    void onChannelState(const mccmsg::StatChannel& state);

private:
    void init();

    void reqAllProtocols();
    void reqProtocol(const mccmsg::Protocol& protocol);
    void onProtocolDescription(const mccmsg::ProtocolDescription& description);

    void reqAllChannels();
    void reqChannel(const mccmsg::Channel& channel);
    void onChannelDescription(const mccmsg::ChannelDescription& description);
    void updateChannelActivation(const mccmsg::Channel& channel, bool isActive);

    UnknownUavs _unknowns;
    ChannelInformations _channels;
    mccmsg::ProtocolDescriptions    _protocols;
    Rc<mccuav::ExchangeService>     _exchangeService;
    mccuav::Rc<mccui::SettingsWriter> _activeChannelsWriter;

    Q_DISABLE_COPY(ChannelsController)
};

class MCC_UAV_DECLSPEC ChannelsControllerPluginData : public mccplugin::PluginData {
public:
    static constexpr const char* id = "mcc::ChannelsControllerPluginData";

    ChannelsControllerPluginData(ChannelsController* channelsController);
    ~ChannelsControllerPluginData();

    ChannelsController* channelsController();
    const ChannelsController* channelsController() const;

private:
    Rc<ChannelsController> _channelsController;
};
}
