#pragma once
#include "mcc/Config.h"
#include <string>
#include <bmcl/Option.h>
#include "mcc/msg/Objects.h"

namespace caf { class actor; }
namespace caf { class actor_config; }
namespace caf { class event_based_actor; }

namespace mccnet {

struct GroupDeviceState
{
    GroupDeviceState(bmcl::Option<std::size_t> groupId, bmcl::Option<std::size_t> leaderId, std::size_t term) : groupId(groupId), leaderId(leaderId), term(term) {}
    bmcl::Option<std::size_t> groupId;
    bmcl::Option<std::size_t> leaderId;
    std::size_t term;
};

struct GroupState
{
    GroupState(std::size_t groupId, std::size_t term, bmcl::Option<std::size_t> leaderId, std::vector<std::size_t> memberIds)
        : groupId(groupId), leaderId(leaderId), memberIds(memberIds), term(term) {}
    std::size_t groupId;
    bmcl::Option<std::size_t> leaderId;
    std::vector<std::size_t> memberIds;
    std::size_t term;
};

MCC_PLUGIN_NET_DECLSPEC caf::actor createGroupActor(caf::event_based_actor* spawer, const caf::actor& core);

}
