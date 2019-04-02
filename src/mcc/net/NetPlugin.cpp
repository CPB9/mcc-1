#include "mcc/net/NetPlugin.h"

namespace mccnet {

NetPlugin::NetPlugin(const mccmsg::ProtocolDescription& dscr) : mccplugin::Plugin(NetPlugin::NetPlugin::id), _description(dscr) {}
NetPlugin::~NetPlugin() {}
const mccmsg::Protocol& NetPlugin::protocol() const { return _description->name(); }
const mccmsg::ProtocolDescription& NetPlugin::description() const { return _description; }
mccmsg::PropertyDescriptionPtrs NetPlugin::getOptionalProperties() const { return mccmsg::PropertyDescriptionPtrs(); }
mccmsg::PropertyDescriptionPtrs NetPlugin::getRequiredProperties() const { return mccmsg::PropertyDescriptionPtrs(); }


}
