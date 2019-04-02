#pragma once
#include "mcc/Config.h"
#include "mcc/msg/FwdExt.h"
#include "mcc/msg/Msg.h"
#include "mcc/msg/Tm.h"
#include "mcc/msg/Cmd.h"

#include <bmcl/Option.h>

namespace mccmsg {

class MCC_MSG_DECLSPEC GroupState
{
public:
    GroupState(const Group& group, bmcl::Option<Device> leader, const Devices& members, const DeviceIds& unknown);
    GroupState(const Device& device, const Group& group, bmcl::Option<Device> leader, Devices&& members, DeviceIds&& unknown);
    const Group& group() const;
    const bmcl::Option<Device>& leader() const;
    const Devices& members() const;
    const DeviceIds& unknown_members() const;
    bool hasMember(const Device& id) const;
    void remove_member(const Device& id);

    bool operator==(const GroupState& other) const;
    bool operator!=(const GroupState& other) const;

private:
    Group _group;
    bmcl::Option<Device>  _leader;
    Devices   _members;
    DeviceIds _unknown_members;
};

class MCC_MSG_DECLSPEC TmGroupState : public TmAny
{
public:
    TmGroupState(const Device& device, const GroupState& state);
    void visit(TmVisitor* visitor) const override;
    const GroupState& state() const;
private:
    GroupState _state;
};

class MCC_MSG_DECLSPEC CmdGroupNew : public DevReq
{
public:
    CmdGroupNew(const Group& group, const Devices& members);
    CmdGroupNew(const Group& group, Devices&& members);
    ~CmdGroupNew();
    void visit(CmdVisitor* visitor) const override;
    const char* nameXXX() const override;
    const char* info() const override;
    const Devices& members() const;
private:
    Devices _members;
};

class MCC_MSG_DECLSPEC CmdGroupDelete : public DevReq
{
public:
    CmdGroupDelete(const Group& group);
    ~CmdGroupDelete();
    void visit(CmdVisitor* visitor) const override;
    const char* nameXXX() const override;
    const char* info() const override;
};

class MCC_MSG_DECLSPEC CmdGroupAttach : public DevReq
{
public:
    CmdGroupAttach(const Group& group, const Device& device);
    ~CmdGroupAttach();
    void visit(CmdVisitor* visitor) const override;
    const char* nameXXX() const override;
    const char* info() const override;
    const Device& attachable() const;
private:
    Device _attachable;
};

class MCC_MSG_DECLSPEC CmdGroupDetach : public DevReq
{
public:
    CmdGroupDetach(const bmcl::Option<Group>& group, const Device& device);
    ~CmdGroupDetach();
    void visit(CmdVisitor* visitor) const override;
    const char* nameXXX() const override;
    const char* info() const override;
    const Device& detachable() const;
private:
    Device _detachable;
};

class MCC_MSG_DECLSPEC CmdGroupSwitch : public DevReq
{
public:
    CmdGroupSwitch(const Device& device, const Group& group, const Group& to);
    ~CmdGroupSwitch();
    void visit(CmdVisitor* visitor) const override;
    const char* nameXXX() const override;
    const char* info() const override;
    const Group& to() const;
private:
    Group _to;
};

}
