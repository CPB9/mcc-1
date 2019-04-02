#include "mcc/msg/GroupState.h"

namespace mccmsg {

GroupState::GroupState(const Group& group, bmcl::Option<Device> leader, const Devices& members, const DeviceIds& unknown)
    : _group(group), _leader(leader), _members(members), _unknown_members(unknown)
{
}
GroupState::GroupState(const Device& device, const Group& group, bmcl::Option<Device> leader, Devices&& members, DeviceIds&& unknown)
    : _group(group), _leader(std::move(leader)), _members(std::move(members)), _unknown_members(std::move(unknown))
{
}
const Group& GroupState::group() const { return _group; }
const bmcl::Option<Device>& GroupState::leader() const { return _leader; }
const Devices& GroupState::members() const { return _members; }
const DeviceIds& GroupState::unknown_members() const { return _unknown_members; }
bool GroupState::hasMember(const Device& id) const
{
    const auto i = std::find_if(_members.begin(), _members.end(), [&](const Device& d) {return d == id; });
    return i != _members.end();
}
void GroupState::remove_member(const Device& id)
{
    const auto i = std::find_if(_members.begin(), _members.end(), [&](const Device& d){return d == id;});
    if (i == _members.end())
        return;
    _members.erase(i);
}

bool GroupState::operator==(const GroupState& other) const
{
    return (_group            == other._group &&
            _leader           == other._leader &&
            _members          == other._members &&
            _unknown_members  == other._unknown_members);
}
bool GroupState::operator!=(const GroupState& other) const
{
    return (_group            != other._group ||
            _leader           != other._leader ||
            _members          != other._members ||
            _unknown_members  != other._unknown_members);
}

TmGroupState::TmGroupState(const Device& device, const GroupState& state) : TmAny(device), _state(state) {}
const GroupState& TmGroupState::state() const { return _state; }

}
