#include <caf/event_based_actor.hpp>
#include "mcc/net/NetLoggerInf.h"
#include "mcc/net/db/DbHandler.h"
#include "mcc/net/db/obj/Firmware.h"
#include "mcc/net/db/obj/Protocol.h"
#include "mcc/net/db/obj/Channel.h"
#include "mcc/net/db/obj/Device.h"
#include "mcc/net/db/obj/DeviceUi.h"
#include "mcc/net/db/obj/Radar.h"
#include "mcc/net/db/obj/TmSession.h"
#include "mcc/net/db/obj/Advanced.h"
#include "mcc/msg/ptr/Firmware.h"
#include "mcc/msg/ptr/Device.h"
#include "mcc/plugin/PluginCache.h"

CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::NotificationPtr)

namespace mccdb {
namespace dbobj {

struct PreparedQueries
{
    explicit PreparedQueries(DbObjInternal* db)
        : _advanced(db)
        , _channel(db)
        , _device(db)
        , _deviceUi(db)
        , _firmware(db)
        , _protocol(db)
        , _radar(db)
        , _tmSession(db)
    {
    }
    dbobj::Advanced _advanced;
    dbobj::Channel _channel;
    dbobj::Device _device;
    dbobj::DeviceUi _deviceUi;
    dbobj::Firmware _firmware;
    dbobj::Protocol _protocol;
    dbobj::Radar _radar;
    dbobj::TmSession _tmSession;
};


sqlite3pp::database& DbObjInternal::db() { return _dbpp.db();}
caf::event_based_actor* DbObjInternal::self() { return _self; }

DbObjInternal::DbObjInternal(caf::event_based_actor* self) : _self(self)
{
    _gr = self->home_system().groups().get_local("notes");
    self->join(_gr);
}

DbObjInternal::~DbObjInternal()
{
    close();
}

void DbObjInternal::close()
{
    _queries.reset();
    _dbpp.close();
}

bool DbObjInternal::open(const std::string& mccDbPath, const std::string& dbSchema)
{
    if (!_dbpp.open(mccDbPath, dbSchema))
        return false;

    _dbpp.db().enable_foreign_keys();
    _dbpp.db().synchronous_mode(sqlite3pp::synchronous::Full);
    _dbpp.db().enable_extended_result_codes();

    _queries = std::make_shared<PreparedQueries>(this);
    return true;
}

void DbObjInternal::registered(const mccmsg::Channel& name, bool state) { _self->send(_gr, mccmsg::makeNote(new mccmsg::channel::Registered(name, state))); }
void DbObjInternal::registered(const mccmsg::Device& name, bool state) { _self->send(_gr, mccmsg::makeNote(new mccmsg::device::Registered(name, state))); }
void DbObjInternal::registered(const mccmsg::Firmware& name, bool state) { _self->send(_gr, mccmsg::makeNote(new mccmsg::firmware::Registered(name, state))); }
void DbObjInternal::registered(const mccmsg::Protocol& name, bool state) { _self->send(_gr, mccmsg::makeNote(new mccmsg::protocol::Registered(name, state))); }
void DbObjInternal::registered(const mccmsg::Radar& name, bool state) { _self->send(_gr, mccmsg::makeNote(new mccmsg::radar::Registered(name, state))); }
void DbObjInternal::registered(const mccmsg::TmSession& name, bool state) { _self->send(_gr, mccmsg::makeNote(new mccmsg::tmSession::Registered(name, state))); }
void DbObjInternal::registered(const mccmsg::DeviceUi& name, bool state) { _self->send(_gr, mccmsg::makeNote(new mccmsg::deviceUi::Registered(name, state))); }

void DbObjInternal::updated(const mccmsg::ChannelDescription& name) { _self->send(_gr, mccmsg::makeNote(new mccmsg::channel::Updated(name))); }
void DbObjInternal::updated(const mccmsg::DeviceDescription& name) { _self->send(_gr, mccmsg::makeNote(new mccmsg::device::Updated(name))); }
void DbObjInternal::updated(const mccmsg::DeviceUiDescription& name) { _self->send(_gr, mccmsg::makeNote(new mccmsg::deviceUi::Updated(name))); }
void DbObjInternal::updated(const mccmsg::FirmwareDescription& name) { _self->send(_gr, mccmsg::makeNote(new mccmsg::firmware::Updated(name))); }
void DbObjInternal::updated(const mccmsg::ProtocolDescription& name) { _self->send(_gr, mccmsg::makeNote(new mccmsg::protocol::Updated(name))); }
void DbObjInternal::updated(const mccmsg::RadarDescription& name) { _self->send(_gr, mccmsg::makeNote(new mccmsg::radar::Updated(name))); }
void DbObjInternal::updated(const mccmsg::TmSessionDescription& name) { _self->send(_gr, mccmsg::makeNote(new mccmsg::tmSession::Updated(name))); }

void DbObjInternal::deviceConnected(bool isConnect, const mccmsg::Device& device, const mccmsg::Channel& channel)
{
    _self->send(_gr, mccmsg::makeNote(new mccmsg::device::Connected(isConnect, device, channel)));
}

void DbObjInternal::setTmSession(const std::string& folder)
{
    _self->send(_gr, mccnet::log_set_atom::value, folder);
}

dbobj::Device& DbObjInternal::device() { return _queries->_device; }
dbobj::DeviceUi& DbObjInternal::deviceUi() { return _queries->_deviceUi; }
dbobj::Protocol& DbObjInternal::protocol() { return _queries->_protocol; }
dbobj::Firmware& DbObjInternal::firmware() { return _queries->_firmware; }
dbobj::Channel& DbObjInternal::channel() { return _queries->_channel; }
dbobj::Radar& DbObjInternal::radar() { return _queries->_radar; }
dbobj::TmSession& DbObjInternal::tmSession() { return _queries->_tmSession; }
dbobj::Advanced& DbObjInternal::advanced() { return _queries->_advanced; }

}
}
