#pragma once
#include "mcc/Config.h"
#include <bmcl/Uuid.h>
#include <bmcl/Fwd.h>

class QUuid;

namespace mccmsg
{
    using DeviceId = std::size_t;
    using GroupId = std::size_t;
}

namespace mccmsg{

#define MAKE_OBJ(name)                  \
    class MCC_MSG_DECLSPEC name : public bmcl::Uuid \
    {                                   \
    public:                             \
        using bmcl::Uuid::Uuid;         \
        explicit name();                \
        explicit name(const Uuid& uuid); \
        const Uuid& uuid() const;       \
        static name generate();         \
        static name createNil();        \
        static name createOrNil(const char* str); \
    };

MAKE_OBJ(Device)
MAKE_OBJ(Group)
MAKE_OBJ(DeviceUi)
MAKE_OBJ(Protocol)
MAKE_OBJ(Channel)
MAKE_OBJ(Radar)
MAKE_OBJ(Firmware)
MAKE_OBJ(TmSession)
MAKE_OBJ(TmExtension)
MAKE_OBJ(Property)

#undef MAKE_OBJ

MCC_MSG_DECLSPEC GroupId groupToId(const Group& group);
MCC_MSG_DECLSPEC Group idToGroup(GroupId id);
MCC_MSG_DECLSPEC bmcl::Option<Group> idToGroup(bmcl::Option<GroupId>& id);

class MCC_MSG_DECLSPEC DeviceOrGroup
{
public:
    DeviceOrGroup(const Device& device);
    DeviceOrGroup(const Group& group);
    bmcl::Option<Device> device() const;
    bmcl::Option <Group> group() const;
    bool isDevice() const;
    bool isGroup() const;
private:
    bmcl::Uuid _uuid;
    bool _isDevice;
};

}
