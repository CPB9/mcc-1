#pragma once
#include <caf/atom.hpp>
#include <map>
#include <fstream>
#include <memory>
#include <caf/actor.hpp>
#include <caf/event_based_actor.hpp>
#include <bmcl/TimeUtils.h>
#include <bmcl/Logging.h>
#include "mcc/msg/Packet.h"
#include "mcc/msg/Stats.h"
#include "mcc/net/Timer.h"


namespace mccnet {

using activated_atom = caf::atom_constant<caf::atom("act")>;
using deactivated_atom = caf::atom_constant<caf::atom("deact")>;
using connected_atom = caf::atom_constant<caf::atom("conn")>;
using disconnected_atom = caf::atom_constant<caf::atom("disconn")>;
using activated_list_atom = caf::atom_constant<caf::atom("actlist")>;
using req_atom = caf::atom_constant<caf::atom("request")>;
using resp_atom = caf::atom_constant<caf::atom("response")>;

using atom_sent = caf::atom_constant<caf::atom("sent")>;
using atom_rcvd = caf::atom_constant<caf::atom("rcvd")>;
using atom_rcvd_bad = caf::atom_constant<caf::atom("rcvdbad")>;
using atom_timeout = caf::atom_constant<caf::atom("timeout")>;

using log_set_atom = caf::atom_constant<caf::atom("logset")>;
using log_start_atom = caf::atom_constant<caf::atom("logstart")>;
using log_finish_atom = caf::atom_constant<caf::atom("logfinish")>;

using group_dev = caf::atom_constant<caf::atom("grdev")>;
using group_cmd = caf::atom_constant<caf::atom("grcmd")>;
using group_cmd_new = caf::atom_constant<caf::atom("grcmdnew")>;
using group_cmd_del = caf::atom_constant<caf::atom("grcmddel")>;
using group_cmd_att = caf::atom_constant<caf::atom("grcmdatt")>;
using group_cmd_det = caf::atom_constant<caf::atom("grcmddet")>;

using group_stat = caf::atom_constant<caf::atom("grst")>;
using group_stat_dev = caf::atom_constant<caf::atom("grstdev")>;

using send_cmd_atom = caf::atom_constant<caf::atom("sndcmd")>;

class MCC_PLUGIN_NET_DECLSPEC ILogWriter
{
public:
    virtual ~ILogWriter();
    virtual bool isOpen() const = 0;
    virtual bmcl::SystemTime startTime() const = 0;
    virtual void open(bmcl::StringView folder, bmcl::SystemTime time) = 0;
    virtual void close(std::chrono::milliseconds time) = 0;
    virtual void rcvd(std::chrono::milliseconds time, mccmsg::PacketPtr& p) = 0;
    virtual void rcvdBad(std::chrono::milliseconds time, mccmsg::PacketPtr& p) = 0;
    virtual void sent(std::chrono::milliseconds time, mccmsg::PacketPtr& p) = 0;
};
using ILogWriterPtr = std::unique_ptr<ILogWriter>;
using LogWriteCreator = std::function<ILogWriterPtr()>;

class MCC_PLUGIN_NET_DECLSPEC DefaultLogWriter : public ILogWriter
{
public:
    DefaultLogWriter(const mccmsg::Channel&);
    ~DefaultLogWriter() override;
    bool isOpen() const override;
    bmcl::SystemTime startTime() const override;
    void open(bmcl::StringView folder, bmcl::SystemTime time) override;
    void close(std::chrono::milliseconds time) override;
    void sent(std::chrono::milliseconds time, mccmsg::PacketPtr& p) override;
    void rcvd(std::chrono::milliseconds time, mccmsg::PacketPtr& p) override;
    void rcvdBad(std::chrono::milliseconds time, mccmsg::PacketPtr& p) override;
    static LogWriteCreator creator(const mccmsg::Channel& name);
private:
    std::string     _name;
    mccmsg::Channel _channel;
    std::ofstream   _file;
    bmcl::SystemTime   _time;
};

class MCC_PLUGIN_NET_DECLSPEC LogSender
{
public:
    LogSender(const mccmsg::Channel& channel, const caf::actor& broker, const caf::actor& logger, const LogWriteCreator& creator);
    ~LogSender();
    void changeLog(bool logState, bool isConnected);
    void onRcv(const mccmsg::PacketPtr& p);
    void onRcvBad(const void * start, std::size_t size);
    void onRcvBad(const mccmsg::PacketPtr& pkt);
    void onSent(std::size_t req_id, const mccmsg::PacketPtr& pkt, const caf::error& err);
    void onConnected();
    void onDisconnected(const caf::error& err);
private:
    bool log;
    mccmsg::Channel channel;
    mccnet::Timer timer;
    caf::actor broker;
    caf::actor logger;
    LogWriteCreator _creator;
};

}
