#pragma once
#include <caf/fwd.hpp>
#include <caf/group.hpp>
#include <caf/actor.hpp>
#include "mcc/msg/ptr/Fwd.h"
#include "mcc/net/db/DbHandler.h"
#include "mcc/plugin/Fwd.h"

namespace mccdb {
namespace dbobj {

class Device;
class DeviceUi;
class Protocol;
class Firmware;
class Channel;
class Radar;
class Advanced;
class TmSession;
struct PreparedQueries;

class DbObjInternal
{
public:
    DbObjInternal(caf::event_based_actor* self);
    ~DbObjInternal();

    bool open(const std::string& mccDbPath, const std::string& dbSchema);
    void close();
    sqlite3pp::database& db();
    caf::event_based_actor* self();

    dbobj::Device& device();
    dbobj::DeviceUi& deviceUi();
    dbobj::Protocol& protocol();
    dbobj::Firmware& firmware();
    dbobj::Channel& channel();
    dbobj::Radar& radar();
    dbobj::TmSession& tmSession();
    dbobj::Advanced& advanced();

    void updated(const mccmsg::DeviceDescription&);
    void updated(const mccmsg::DeviceUiDescription&);
    void updated(const mccmsg::ProtocolDescription&);
    void updated(const mccmsg::FirmwareDescription&);
    void updated(const mccmsg::ChannelDescription&);
    void updated(const mccmsg::RadarDescription&);
    void updated(const mccmsg::TmSessionDescription&);

    void registered(const mccmsg::Channel&, bool);
    void registered(const mccmsg::Device&, bool);
    void registered(const mccmsg::Firmware&, bool);
    void registered(const mccmsg::Protocol&, bool);
    void registered(const mccmsg::Radar&, bool);
    void registered(const mccmsg::TmSession&, bool);
    void registered(const mccmsg::DeviceUi&, bool);

    void setTmSession(const std::string&);

    void deviceConnected(bool isConnect, const mccmsg::Device& device, const mccmsg::Channel& channel);
private:
    DbHandler _dbpp;
    caf::group _gr;
    caf::actor _dbtm;
    caf::event_based_actor* _self;
    std::shared_ptr<PreparedQueries> _queries;
};

}
}
