#include "mcc/msg/Objects.h"
#include <bmcl/Option.h>
#include <bmcl/Result.h>
#include <bmcl/StringView.h>

namespace mccmsg{

#define MAKE_OBJ_DEF(name)                              \
    name::name() : bmcl::Uuid(Uuid::createNil()) {}     \
    name::name(const Uuid& uuid) : bmcl::Uuid(uuid) {}  \
    const bmcl::Uuid& name::uuid() const { return *this; }              \
    name name::generate() { return name (bmcl::Uuid::create()); }       \
    name name::createNil() { return name (bmcl::Uuid::createNil()); }   \
    name name::createOrNil(const char* str) { return name(bmcl::Uuid::createFromStringOrNil(bmcl::StringView(str))); }   \

MAKE_OBJ_DEF(Device);
MAKE_OBJ_DEF(Group);
MAKE_OBJ_DEF(DeviceUi)
MAKE_OBJ_DEF(Protocol)
MAKE_OBJ_DEF(Channel)
MAKE_OBJ_DEF(Radar)
MAKE_OBJ_DEF(Firmware)
MAKE_OBJ_DEF(TmSession)
MAKE_OBJ_DEF(TmExtension)
MAKE_OBJ_DEF(Property)

bmcl::Option<Group> idToGroup(bmcl::Option<GroupId>& id)
{
    if (id.isNone())
        return bmcl::None;
    return Group(bmcl::Uuid(id.unwrap(), 0, 0, 0));
}

GroupId groupToId(const Group& group) { return group.part1();}
Group idToGroup(GroupId id) { return Group(id, 0, 0, 0); }

DeviceOrGroup::DeviceOrGroup(const Device& device) : _uuid(device.uuid()), _isDevice(true) {}
DeviceOrGroup::DeviceOrGroup(const Group& group) : _uuid(group.uuid()), _isDevice(false) {}
bmcl::Option<Device> DeviceOrGroup::device() const { if (_isDevice) return Device(_uuid); return bmcl::None; }
bmcl::Option<Group> DeviceOrGroup::group() const { if (!_isDevice) return Group(_uuid); return bmcl::None; }
bool DeviceOrGroup::isDevice() const { return _isDevice; }
bool DeviceOrGroup::isGroup() const { return !_isDevice; }

}
