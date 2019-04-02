#include "mcc/msg/ProtocolController.h"
#include "mcc/net/NetPlugin.h"
#include "mcc/plugin/PluginCache.h"

namespace mccmsg
{

ProtocolController* ProtocolControllerPluginData::controller() { return _controller.get(); }
ProtocolControllerPluginData::~ProtocolControllerPluginData() {}
ProtocolControllerPluginData::ProtocolControllerPluginData(ProtocolController* controller)
    : mccplugin::PluginData(ProtocolControllerPluginData::id)
    , _controller(controller)
{
}

ProtocolController::ProtocolController(mccmsg::ProtocolDescriptions&& dscrs) : _dscrs(std::move(dscrs)) {}
ProtocolController::~ProtocolController() {}
const mccmsg::ProtocolDescriptions& ProtocolController::dscrs() const { return _dscrs; }

}
