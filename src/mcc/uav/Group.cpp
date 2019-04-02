#include "mcc/uav/Group.h"
#include "mcc/uav/Uav.h"

#include <bmcl/Logging.h>

namespace mccuav {

Group::Group(const mccmsg::TmGroupStatePtr& state)
    : _state(state->state())
{}

bool Group::updateState(const mccmsg::TmGroupStatePtr& s)
{
    if(s->state().group() != _state.group()) // alien group
        return false;

    if(s->state() == _state)
        return false;

    _state = s->state();
    return true;
}

void Group::updateGeometry(const mccgeo::GroupGeometry& geometry)
{
    _geometry = geometry;
}

mccmsg::GroupId Group::number() const
{
    return mccmsg::groupToId(_state.group());
}

bool Group::isEmpty() const
{
    return _state.members().empty();
}

size_t Group::size() const
{
    return _state.members().size();
}

bool Group::hasUav(const mccmsg::Device& deviceId) const
{
    return _state.hasMember(deviceId);
}

bool Group::hasUav(const Uav* uav) const
{
    if(uav == nullptr)
        return false;

    return hasUav(uav->device());
}

void Group::removeUav(const mccmsg::Device& deviceId)
{
    _state.remove_member(deviceId);
}

void Group::removeUav(const Uav* uav)
{
    if(uav == nullptr)
        return;

    removeUav(uav->device());
}
}
