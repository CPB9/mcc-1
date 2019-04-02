#include "mcc/uav/ChannelsController.h"
#include "mcc/uav/UavController.h"
#include "mcc/uav/Uav.h"
#include "mcc/uav/ExchangeService.h"
#include "mcc/msg/ptr/Protocol.h"
#include "mcc/msg/ptr/Channel.h"
#include "mcc/ui/Settings.h"

#include <bmcl/OptionRc.h>
#include <QMessageBox>
#include <algorithm>

namespace mccuav {

ChannelsController::ChannelsController(mccui::Settings* settings, mccuav::ExchangeService* exchangeService, QObject* parent)
    : QObjectRefCountable<QObject>(parent)
    , _exchangeService(exchangeService)
{
    _activeChannelsWriter = settings->acquireUniqueWriter("exchange/activeChannels").unwrap();
    connectSlots();
}

ChannelsController::~ChannelsController()
{
}

bmcl::Option<const ChannelInformation&> ChannelsController::channelInformation(const mccmsg::Channel& channel) const
{
    const auto it = std::find_if(_channels.begin(), _channels.end(), [&channel](const ChannelInformation& info) { return info.channel() == channel; });
    if (it == _channels.end())
        return bmcl::None;
    return *it;
}

bmcl::Option<const mccmsg::ProtocolDescription&> ChannelsController::protocolDescription(const mccmsg::Protocol& protocol) const
{
    auto it = std::find_if(_protocols.begin(), _protocols.end(), [&protocol](const mccmsg::ProtocolDescription& desc) { return protocol == desc->name(); });
    if(it == _protocols.end())
        return bmcl::None;
    return *it;
}

const mccmsg::ProtocolDescriptions& ChannelsController::protocols() const
{
    return _protocols;
}

const ChannelInformations& ChannelsController::channelInformations() const
{
    return _channels;
}

bmcl::Option<bool> ChannelsController::isChannelActive(const mccmsg::Channel& channel) const
{
    const auto it = std::find_if(_channels.begin(), _channels.end(), [&channel](const ChannelInformation& info) { return info.channel() == channel; });
    if (it == _channels.end())
        return bmcl::None;
    return it->isActive();
}

void ChannelsController::reqAllChannels()
{
    _exchangeService->requestXX(new mccmsg::channel::DescriptionList_Request).then
    (
        [this](const mccmsg::channel::DescriptionList_ResponsePtr& rep)
        {
            for (const auto& desc : rep->data())
            {
                onChannelDescription(desc);
            }
        }
    );
}

void ChannelsController::requestChannelUnregister(const mccmsg::Channel& id, QWidget* parent)
{
    _exchangeService->requestXX(new mccmsg::channel::UnRegister_Request(id)).then
    (
        [this](const mccmsg::channel::UnRegister_ResponsePtr&)
        {
        },
        [this, parent](const mccmsg::ErrorDscr& err)
        {
            QMessageBox::warning(parent, "Ошибка при удалении канала обмена", "channelRegisterFailed: " + err.qfull());
        }
    );
}

void ChannelsController::requestChannelRegister(const mccmsg::ChannelDescription& description, QWidget* parent)
{
    _exchangeService->requestXX(new mccmsg::channel::Register_Request(description)).then
    (
        [this](const mccmsg::channel::Register_ResponsePtr& rep)
        {
            onChannelDescription(rep->data());
        } ,
        [this, parent](const mccmsg::ErrorDscr& err)
        {
            QMessageBox::warning(parent, "Ошибка при создании канала обмена", "channelRegisterFailed: " + err.qfull());
        }
    );
}

void ChannelsController::reqAllProtocols()
{
    _exchangeService->requestXX(new mccmsg::protocol::DescriptionList_Request).then
    (
        [this](const mccmsg::protocol::DescriptionList_ResponsePtr& rep)
        {
            for (const auto& desc : rep->data())
            {
                onProtocolDescription(desc);
            }
        }
    );
}

void ChannelsController::reqProtocol(const mccmsg::Protocol& protocol)
{
    _exchangeService->requestXX(new mccmsg::protocol::Description_Request(protocol)).then
    (
        [this](const mccmsg::protocol::Description_ResponsePtr& rep)
        {
            onProtocolDescription(rep->data());
        }
    );
}

void ChannelsController::reqChannel(const mccmsg::Channel& channel)
{
    _exchangeService->requestXX(new mccmsg::channel::Description_Request(channel)).then
    (
        [this](const mccmsg::channel::Description_ResponsePtr& rep)
        {
            onChannelDescription(rep->data());
        },
        [this, channel](const mccmsg::ErrorDscr& err)
        {
            //QMessageBox::warning(this, "Ошибка при создании канала обмена", "channelRegisterFailed: " + err.qfull());
        }
    );
}

void ChannelsController::requestChannelActivate(const mccmsg::Channel& channel, bool isEnabled, QWidget* parent)
{
    const auto it = std::find_if(_channels.begin(), _channels.end(), [&channel](const ChannelInformation& info) { return info.channel() == channel; });
    if (it == _channels.end())
        return;

    _exchangeService->requestXX(new mccmsg::channel::Activate_Request(channel, isEnabled)).then
    (
        [this, channel, isEnabled](const mccmsg::channel::Activate_ResponsePtr& rep)
        {
            onChannelActivated(channel, isEnabled);
        },
        [this, channel, parent](const mccmsg::ErrorDscr& err)
        {
            QMessageBox::warning(parent, "Ошибка при открытии канала обмена", "channelActivationFailure: " + channel.toQString() + ", " + err.qfull());

            QStringList values = _activeChannelsWriter->read().toStringList();
            values.removeAll(channel.toQString());
            _activeChannelsWriter->write(values);

            updateChannelActivation(channel, false);
        }
    );
}

void ChannelsController::requestChannelUpdate(const mccmsg::ChannelDescription& description, QWidget* parent)
{
    const mccmsg::Channel& channel = description->name();
    _exchangeService->requestXX(new mccmsg::channel::Update_Request(description)).then
    (
        [this, channel](const mccmsg::channel::Update_ResponsePtr& rep)
        {
            onChannelUpdated(rep->data());
        },
        [this, channel, parent](const mccmsg::ErrorDscr& err)
        {
            QMessageBox::warning(parent, "Ошибка при редактировании канала обмена", "channelUpdateFailed: " + channel.toQString() + ", " + err.qfull());
        }
    );
}

void ChannelsController::setAllChannelsEnabled(bool isEnabled)
{
    for (const auto& chInfo : _channels)
    {
        requestChannelActivate(chInfo.channel(), isEnabled, nullptr);
    }
}

void ChannelsController::init()
{
    reqAllChannels();
    reqAllProtocols();
}

void ChannelsController::connectSlots()
{
    using mccuav::ExchangeService;

    auto service = _exchangeService.get();

    connect(service, &ExchangeService::protocolRegistered, this, &ChannelsController::onProtocolRegistered);

    connect(service, &ExchangeService::channelActivated, this, &ChannelsController::onChannelActivated);
    connect(service, &ExchangeService::channelUpdated, this, &ChannelsController::onChannelUpdated);
    connect(service, &ExchangeService::channelRegistered, this, &ChannelsController::onChannelRegistered);
    connect(service, &ExchangeService::channelUnRegistered, this, &ChannelsController::onChannelUnRegistered);
    connect(service, &ExchangeService::channelState, this, &ChannelsController::onChannelState);

    connect(service, &ExchangeService::deviceConnected, this, &ChannelsController::uavConnected);
    connect(service, &ExchangeService::deviceDisconnected, this, &ChannelsController::uavDisconnected);

    init();
}

void ChannelsController::onProtocolRegistered(const mccmsg::Protocol& protocol)
{
    if (protocolDescription(protocol).isSome())
        return;
    reqProtocol(protocol);
}

void ChannelsController::onProtocolDescription(const mccmsg::ProtocolDescription& description)
{
    if (protocolDescription(description->name()).isSome())
        return;

    _protocols.emplace_back(description);

    emit protocolsChanged();
}

void ChannelsController::onChannelActivated(const mccmsg::Channel& channel, bool isActive)
{
    updateChannelActivation(channel, isActive);

    QStringList values = _activeChannelsWriter->read().toStringList();
    int index = values.indexOf(channel.toQString());
    if (index != -1)
    {
        values.removeAt(index);
    }
    if (isActive)
        values << channel.toQString();

    _activeChannelsWriter->write(values);
}

void ChannelsController::onChannelUpdated(const mccmsg::ChannelDescription& channel)
{
    onChannelDescription(channel);
    emit channelsChanged();
}

void ChannelsController::onChannelRegistered(const mccmsg::Channel& channel)
{
    reqChannel(channel);
}

void ChannelsController::onChannelUnRegistered(const mccmsg::Channel& channel)
{
    const auto it = std::find_if(_channels.begin(), _channels.end(), [&channel](const ChannelInformation& info) { return info.channel() == channel; });
    if (it == _channels.end())
        return;

    _channels.erase(it);
    emit channelsChanged();
    //TODO: remove later (it's for old ConnectionTool)
    emit channelUnRegistered(channel);
}

void ChannelsController::onChannelDescription(const mccmsg::ChannelDescription& description)
{
    const auto it = std::find_if(_channels.begin(), _channels.end(), [&description](const ChannelInformation& info) { return info.channel() == description->name(); });
    if (it == _channels.end())
        _channels.emplace_back(description->name(), description);
    else
        it->setChannelDescription(description);

    emit channelDescription(description->name(), description);

    auto activeChannels = _activeChannelsWriter->read().toStringList();
    if (activeChannels.indexOf(description->name().toQString()) != -1)
    {
        requestChannelActivate(description->name(), true, nullptr);
        //QTimer::singleShot(1000, this, [this, channel]() { _chanController->requestChannelActivate(channel, true, this); });
    }

    emit channelsChanged();
}

void ChannelsController::onChannelState(const mccmsg::StatChannel& state)
{
    // Use only registered channels
    auto i = std::find_if(_channels.begin(), _channels.end(), [&state](const ChannelInformation& info) { return info.channel() == state._channel; });
    if (i == _channels.end())
    {
        reqChannel(state._channel);
        return;
    }

    bool statusChanged = i->stat().isNone() || (i->stat()->_isActive != state._isActive);
    i->setStat(state);
    if (statusChanged)
        channelActiveChanged(i->channel(), i->stat()->_isActive);
    emit channelStatsUpdated(state._channel, state);
}

void ChannelsController::updateChannelActivation(const mccmsg::Channel& channel, bool isActive)
{
    const auto it = std::find_if(_channels.begin(), _channels.end(), [&channel](const ChannelInformation& info) { return info.channel() == channel; });
    if(it == _channels.end())
        return;

    it->setActive(isActive);

    emit channelActiveChanged(channel, isActive);
}

bool ChannelsController::isUavInChannel(const mccmsg::Device& device)
{
    for(const auto& i: _channels)
    {
        if (i.channelDescription().isNone())
            continue;
        const auto& ds = i.channelDescription().unwrap()->connectedDevices();
        const auto j = std::find_if(ds.begin(), ds.end(), [device](const mccmsg::ProtocolId& id){ return id.device() == device;});
        if (j != ds.end())
            return true;
    }
    return false;
}

ChannelsControllerPluginData::ChannelsControllerPluginData(ChannelsController* channelsController)
    : mccplugin::PluginData(id)
    , _channelsController(channelsController)
{
}

ChannelsControllerPluginData::~ChannelsControllerPluginData()
{
}

ChannelsController* ChannelsControllerPluginData::channelsController()
{
    return _channelsController.get();
}

const ChannelsController* ChannelsControllerPluginData::channelsController() const
{
    return _channelsController.get();
}
}

