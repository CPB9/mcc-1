#include <iterator>
#include <sstream>
#include <fmt/format.h>
#include <bmcl/MakeRc.h>
#include <bmcl/OptionSize.h>
#include "mcc/msg/obj/Channel.h"

namespace mccmsg {

INetVisitor::~INetVisitor(){}

fmt::string_view view(bmcl::StringView v) { return fmt::string_view(v.begin(), v.size());}
INetParams::INetParams(NetTransport transport, std::string&& printableName)
    :_transport(transport), _printableName(std::move(printableName)){}
INetParams::~INetParams(){}
NetTransport INetParams::transport() const { return _transport; }
const std::string& INetParams::printableName() const { return _printableName; }

void NetTcpParams::visit(INetVisitor& visitor) const { visitor.visit(*this); }
void NetUdpParams::visit(INetVisitor& visitor) const { visitor.visit(*this); }
void NetSerialParams::visit(INetVisitor& visitor) const { visitor.visit(*this); }

NetTcpParams::NetTcpParams(bmcl::StringView host, uint16_t remotePort)
    : INetParams(NetTransport::Tcp, fmt::format("{}:{}", view(host), remotePort)), _host(host.toStdString()), _remotePort(remotePort){}
NetTcpParams::~NetTcpParams() {}
uint16_t NetTcpParams::remotePort() const { return _remotePort;}
const std::string& NetTcpParams::host() const {return _host;}

NetUdpParams::NetUdpParams(bmcl::StringView host, bmcl::Option<uint16_t> remotePort, bmcl::Option<uint16_t> localPort)
    : INetParams(NetTransport::Udp, fmt::format("{}:{}:{}", remotePort.unwrapOr(uint16_t(0)), localPort.unwrapOr(uint16_t(0)), view(host))), _host(host.toStdString()), _remotePort(remotePort), _localPort(localPort) {}
NetUdpParams::~NetUdpParams() {}
const std::string& NetUdpParams::host() const { return _host; }
const bmcl::Option<uint16_t>& NetUdpParams::remotePort() const { return _remotePort; }
const bmcl::Option<uint16_t>& NetUdpParams::localPort() const { return _localPort; }

NetSerialParams::NetSerialParams(bmcl::StringView portName, std::size_t baudRate)
    : INetParams(NetTransport::Serial, fmt::format("{}:{}", baudRate, view(portName))), _portName(portName.toStdString()), _baudRate(baudRate) {}
NetSerialParams::~NetSerialParams() {}
std::size_t NetSerialParams::baudRate() const {return _baudRate;}
const std::string& NetSerialParams::portName() const { return _portName; }

ChannelDescriptionObj::~ChannelDescriptionObj() {}
const Channel& ChannelDescriptionObj::name() const { return _name; }
const Protocol& ChannelDescriptionObj::protocol() const { return _protocol; }
const std::string& ChannelDescriptionObj::info() const { return _info; }
const bmcl::Option<Radar>& ChannelDescriptionObj::radar() const { return _radar; }
const ProtocolIds& ChannelDescriptionObj::connectedDevices() const { return _connectedDevices; }
const std::chrono::milliseconds& ChannelDescriptionObj::timeout() const { return _timeout; }
bool ChannelDescriptionObj::log() const { return _log; }
bool ChannelDescriptionObj::isDynamicTimeout() const { return _isDynamicTimeout; }
bool ChannelDescriptionObj::isReadOnly() const { return _isReadOnly; }
const bmcl::Option<std::chrono::seconds>& ChannelDescriptionObj::reconnect() const { return _reconnect; }

const INetPtr& ChannelDescriptionObj::params() const { return _params; }

ChannelDescriptionObj::ChannelDescriptionObj(const Channel& name
    , const Protocol& protocol
    , bmcl::StringView info
    , const INetPtr& net
    , bool log
    , const std::chrono::milliseconds& timeout
    , bool isDynamicTimeout
    , bool isReadOnly
    , const bmcl::Option<std::chrono::seconds>& reconnect
    , const bmcl::Option<Radar>& radar
    , const ProtocolIds& connectedDevices)
    : _name(name)
    , _protocol(protocol)
    , _info(info.toStdString())
    , _log(log)
    , _timeout(timeout)
    , _isReadOnly(isReadOnly)
    , _isDynamicTimeout(isDynamicTimeout)
    , _reconnect(reconnect)
    , _radar(radar)
    , _connectedDevices(connectedDevices)
    , _params(net)
{
}

std::vector<bmcl::StringView> split(bmcl::StringView s, char delim)
{
    if (s.isEmpty())
        return std::vector<bmcl::StringView>();
    std::vector<bmcl::StringView> elems;

    std::size_t start = 0;

    do
    {
        auto end = s.findFirstOf(delim, start);
        elems.emplace_back(s.slice(start, end.unwrapOr(s.size())));
        start = end.unwrapOr(s.size()) + 1;
    } while (start < s.size());
    return elems;
}

std::string NetTcpParams::to_string() const { return fmt::format("tcp:{}:{}", _host, _remotePort); }
std::string NetUdpParams::to_string() const { return fmt::format("udp:{}:{}:{}", _remotePort.unwrapOr(uint16_t(0)), _localPort.unwrapOr(uint16_t(0)), _host); }
std::string NetSerialParams::to_string() const { return fmt::format("com:{}:{}", _baudRate, _portName); }

ChannelDescription from_string(const Channel& name
    , const Protocol& protocol
    , bmcl::StringView info
    , bmcl::StringView settings
    , bool log
    , const std::chrono::milliseconds& timeout
    , bool isDynamicTimeout
    , bool isReadOnly
    , const bmcl::Option<std::chrono::seconds>& reconnect
    , const bmcl::Option<Radar>& radar
    , const ProtocolIds& connectedDevices)
{
    INetPtr net;
    if (settings.startsWith("udp"))
    {
        net = NetUdpParams::from_string(settings).unwrapOr(nullptr);
    }
    else if (settings.startsWith("tcp"))
    {
        net = NetTcpParams::from_string(settings).unwrapOr(nullptr);
    }
    else if (settings.startsWith("com"))
    {
        net = NetSerialParams::from_string(settings).unwrapOr(nullptr);
    }
    if (net.isNull())
        return nullptr;
    return bmcl::makeRc<const ChannelDescriptionObj>(name, protocol, info, net, log, timeout, isDynamicTimeout, isReadOnly, reconnect, radar, connectedDevices);
}

bmcl::OptionRc<const NetTcpParams> NetTcpParams::from_string(bmcl::StringView s)
{
    auto r = split(s, ':');
    if (r.size() != 3 || r[0] != "tcp") return bmcl::None;
    return bmcl::makeRc<const NetTcpParams>(r[2], (uint16_t)atoi(r[1].toStdString().c_str()));
}

bmcl::OptionRc<const NetUdpParams> NetUdpParams::from_string(bmcl::StringView s)
{
    auto r = split(s, ':');
    if (r.size() != 4 || r[0] != "udp") return bmcl::None;
    bmcl::Option<uint16_t> remote;
    bmcl::Option<uint16_t> local;
    if (std::atoi(r[1].toStdString().c_str()) != 0) remote = (uint16_t)std::atoi(r[1].toStdString().c_str());
    if (std::atoi(r[2].toStdString().c_str()) != 0) local = (uint16_t)std::atoi(r[2].toStdString().c_str());
    return bmcl::makeRc<const NetUdpParams>(r[3], remote, local);
}

bmcl::OptionRc<const NetSerialParams> NetSerialParams::from_string(bmcl::StringView s)
{
    auto r = split(s, ':');
    if (r.size() != 3 || r[0] != "com") return bmcl::None;
    return bmcl::makeRc<const NetSerialParams>(r[2], (std::size_t)std::atoll(r[1].toStdString().c_str()));
}

}
