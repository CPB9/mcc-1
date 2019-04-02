#pragma once
#include "mcc/Config.h"
#include <chrono>
#include <vector>
#include <bmcl/Rc.h>
#include <bmcl/Option.h>
#include <bmcl/OptionRc.h>
#include <bmcl/StringView.h>
#include "mcc/Rc.h"
#include "mcc/msg/obj/Protocol.h"

namespace mccmsg {

enum class NetTransport : uint8_t
{
    Unknown,
    Tcp,
    Udp,
    Serial,
};

class INetVisitor;

class MCC_MSG_DECLSPEC INetParams : public mcc::RefCountable
{
public:
    INetParams(NetTransport transport, std::string&& printableName);
    virtual ~INetParams();
    virtual void visit(INetVisitor& net) const = 0;
    virtual std::string to_string() const = 0;
    NetTransport transport() const;
    const std::string& printableName() const;
private:
    NetTransport _transport;
    std::string  _printableName;
};
using INetPtr = bmcl::Rc<const INetParams>;

class MCC_MSG_DECLSPEC NetTcpParams : public INetParams
{
public:
    NetTcpParams(bmcl::StringView host, uint16_t remotePort);
    ~NetTcpParams() override;
    void visit(INetVisitor& net) const override;
    std::string to_string() const override;
    static bmcl::OptionRc<const NetTcpParams> from_string(bmcl::StringView s);
    uint16_t remotePort() const;
    const std::string& host() const;
private:
    uint16_t _remotePort;
    std::string _host;
};
using NetTcpPtr = bmcl::Rc<const NetTcpParams>;

class MCC_MSG_DECLSPEC NetUdpParams : public INetParams
{
public:
    NetUdpParams(bmcl::StringView host, bmcl::Option<uint16_t> remotePort, bmcl::Option<uint16_t> localPort);
    ~NetUdpParams() override;
    void visit(INetVisitor& net) const override;
    std::string to_string() const override;
    static bmcl::OptionRc<const NetUdpParams> from_string(bmcl::StringView s);
    const std::string& host() const;
    const bmcl::Option<uint16_t>& remotePort() const;
    const bmcl::Option<uint16_t>& localPort() const;
private:
    std::string _host;
    bmcl::Option<uint16_t> _remotePort;
    bmcl::Option<uint16_t> _localPort;
};
using NetUdpPtr = bmcl::Rc<const NetUdpParams>;

class MCC_MSG_DECLSPEC NetSerialParams : public INetParams
{
public:
    NetSerialParams(bmcl::StringView portName, std::size_t baudRate);
    ~NetSerialParams() override;
    void visit(INetVisitor& net) const override;
    std::string to_string() const override;
    static bmcl::OptionRc<const NetSerialParams> from_string(bmcl::StringView s);
    std::size_t baudRate() const;
    const std::string &portName() const;
public:
    std::size_t _baudRate;
    std::string _portName;
};
using NetSerialPtr = bmcl::Rc<const NetSerialParams>;

class MCC_MSG_DECLSPEC INetVisitor
{
public:
    virtual ~INetVisitor();
    virtual void visit(const NetTcpParams&) = 0;
    virtual void visit(const NetUdpParams&) = 0;
    virtual void visit(const NetSerialParams&) = 0;
};

class MCC_MSG_DECLSPEC ChannelDescriptionObj : public mcc::RefCountable
{
public:
    ChannelDescriptionObj(const Channel& name
        , const Protocol& protocol
        , bmcl::StringView info
        , const INetPtr& net
        , bool log
        , const std::chrono::milliseconds& timeout
        , bool isDynamicTimeout
        , bool isReadOnly
        , const bmcl::Option<std::chrono::seconds>& reconnect
        , const bmcl::Option<Radar>& radar
        , const ProtocolIds& connectedDevices = {});

    ~ChannelDescriptionObj() override;
    const Channel& name() const;
    const Protocol& protocol() const;
    const std::string& info() const;
    const bmcl::Option<Radar>& radar() const;
    const ProtocolIds& connectedDevices() const;
    const std::chrono::milliseconds& timeout() const;
    bool log() const;
    bool isDynamicTimeout() const;
    bool isReadOnly() const;
    const bmcl::Option<std::chrono::seconds>& reconnect() const;
    const INetPtr& params() const;

private:
    Channel _name;
    Protocol _protocol;
    std::string _info;
    bmcl::Option<Radar> _radar;
    ProtocolIds _connectedDevices;
    INetPtr _params;
    bool _log;
    bool _isReadOnly;
    bool _isDynamicTimeout;
    std::chrono::milliseconds _timeout;
    bmcl::Option<std::chrono::seconds> _reconnect;
};

using Channels = std::vector<Channel>;
using ChannelDescription = bmcl::Rc<const ChannelDescriptionObj>;
using ChannelDescriptions = std::vector<ChannelDescription>;

MCC_MSG_DECLSPEC ChannelDescription from_string(const Channel& name
    , const Protocol& protocol
    , bmcl::StringView info
    , bmcl::StringView net
    , bool log
    , const std::chrono::milliseconds& timeout
    , bool isDynamicTimeout
    , bool isReadOnly
    , const bmcl::Option<std::chrono::seconds>& reconnect
    , const bmcl::Option<Radar>& radar
    , const ProtocolIds& connectedDevices = {});

}
