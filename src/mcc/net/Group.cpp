#include <fmt/format.h>
#include <caf/actor_config.hpp>
#include <caf/event_based_actor.hpp>
#include <bmcl/Result.h>
#include <bmcl/Rc.h>
#include <bmcl/OptionRc.h>
#include <bmcl/RefCountable.h>
#include "mcc/msg/ptr/Tm.h"
#include "mcc/msg/ptr/ReqVisitor.h"
#include "mcc/msg/ptr/Protocol.h"
#include "mcc/net/Group.h"
#include "mcc/net/NetLoggerInf.h"
#include "mcc/net/Cmd.h"

CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::Group);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::DevReqPtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::NotificationPtr);

namespace mccnet {

using timerAtom = caf::atom_constant<caf::atom("timer")>;

class GroupHandler;

struct Group : public mcc::RefCountable
{
    Group(const mccmsg::Device& device, std::size_t term, const mccmsg::Group& group, const bmcl::Option<mccmsg::Device>& leader)
        : _term(term), _group(group), _leader(leader)
    {
        _confirmed_devices.insert(device);
    }
    ~Group() {}
    bool update(const mccmsg::Device& device, std::size_t term, const bmcl::Option<mccmsg::Device>& leader)
    {
        if (term != 0 && _term > term)
            return false;

        _term = term;
        _leader = leader;
        _confirmed_devices.insert(device);
        return true;
    }
    void remove_confirmed(const mccmsg::Device& device)
    {
        _confirmed_devices.erase(device);
    }
    void add_confirmed(const mccmsg::Device& device)
    {
        _confirmed_devices.insert(device);
    }
    std::size_t _term;
    mccmsg::Group _group;
    bmcl::Option<mccmsg::Device> _leader;
    std::set<mccmsg::Device>     _confirmed_devices;
};
using GroupPtr = bmcl::Rc<Group>;

struct Dev : public mcc::RefCountable
{
    Dev(const mccmsg::ProtocolId& id, const caf::actor& a, const bmcl::OptionRc<Group>& group, std::size_t term)
        : id(id), a(a), _group(group), term(term) {}
    ~Dev() { }
    void update_my_group(const bmcl::OptionRc<Group>& group)
    {
        if (_group == group)
            return;
        if (_group.isSome())
        {
            _group.unwrap()->remove_confirmed(id.device());
        }
        _group = group;
        if (_group.isSome())
        {
            _group.unwrap()->add_confirmed(id.device());
        }
    }

    mccmsg::ProtocolId id;
    caf::actor a;
    bmcl::OptionRc<Group> _group;
    std::size_t term;
};
using DevPtr = bmcl::Rc<Dev>;

class GroupHandler : public caf::event_based_actor
{
    friend class VisitorGroup;
public:
    GroupHandler(caf::actor_config& cfg, const caf::actor& core, const std::string& name);
    caf::behavior make_behavior() override;
    const char* name() const override;
    void on_exit() override;
private:
    void execute(CmdPtr&& cmd, const mccmsg::CmdGroupNew& msg);
    void execute(CmdPtr&& cmd, const mccmsg::CmdGroupDelete& msg);
    void execute(CmdPtr&& cmd, const mccmsg::CmdGroupAttach& msg);
    void execute(CmdPtr&& cmd, const mccmsg::CmdGroupDetach& msg);
    void execute(CmdPtr&& cmd, const mccmsg::CmdGroupSwitch& msg);
    void execute(CmdPtr&& cmd);

    GroupPtr getGroup(const mccmsg::Device& device, std::size_t term, const mccmsg::Group& group_name, const bmcl::Option<mccmsg::Device>& leader);
    bmcl::Option<mccmsg::Device> getDevice(bmcl::Option<mccmsg::DeviceId>) const;
    void deleteDevice(const mccmsg::Device&);

    using Devs = std::map<mccmsg::Device, DevPtr>;
    Devs _devices;

    using Groups = std::map<mccmsg::Group, GroupPtr>;
    Groups _groups;

    bmcl::Result<DevPtr, caf::error> getLeader(const bmcl::Option<mccmsg::Group>& group);

    std::string _name;
    caf::actor _core;
};

class VisitorGroup : public mccmsg::CmdVisitor
{
public:
    VisitorGroup(GroupHandler* self, CmdPtr&& cmd);

    using mccmsg::CmdVisitor::visit;

    void visit(const mccmsg::CmdGroupNew* msg) override { /*(void)msg; assert(false);*/ }
    void visit(const mccmsg::CmdGroupDelete* msg) override { _self->execute(std::move(_cmd), *msg); }
    void visit(const mccmsg::CmdGroupAttach* msg) override { _self->execute(std::move(_cmd), *msg); }
    void visit(const mccmsg::CmdGroupDetach* msg) override { _self->execute(std::move(_cmd), *msg); }
    void visit(const mccmsg::CmdGroupSwitch* msg) override { _self->execute(std::move(_cmd), *msg); }
private:
    GroupHandler* _self;
    CmdPtr _cmd;
};

VisitorGroup::VisitorGroup(GroupHandler* self, CmdPtr&& cmd)
    : mccmsg::CmdVisitor([&](const mccmsg::DevReq*) { _self->execute(std::move(_cmd)); })
    , _self(self)
    , _cmd(std::move(cmd))
{
}

GroupHandler::GroupHandler(caf::actor_config& cfg, const caf::actor& core, const std::string& name) : caf::event_based_actor(cfg), _name(name), _core(core)
{
}

const char* GroupHandler::name() const
{
    return _name.c_str();
}

void GroupHandler::on_exit()
{
    _groups.clear();
    _devices.clear();
    destroy(_core);
}

GroupPtr GroupHandler::getGroup(const  mccmsg::Device& device, std::size_t term, const mccmsg::Group& group_name, const bmcl::Option<mccmsg::Device>& leader)
{
    Groups::iterator i = _groups.find(group_name);
    if (i != _groups.end())
        return i->second;

    GroupPtr p = new Group(device, term, group_name, leader);
    _groups[group_name] = p;
    return p;
}

caf::behavior GroupHandler::make_behavior()
{
    set_down_handler([this](const caf::down_msg& dm)
    {
        auto i = std::find_if(_devices.begin(), _devices.end(), [&dm](const typename Devs::value_type& i) { return i.second->a == dm.source; });
        if (i != _devices.end())
            deleteDevice(i->second->id.device());
    });

    send(this, timerAtom::value);

    return
    {
        [this](const mccmsg::DevReqPtr& cmd)
        {
            VisitorGroup visitor{ this, new mccnet::Cmd(make_response_promise(), cmd, mccmsg::Error::GroupUnreachable) };
            cmd->visit(&visitor);
            return caf::delegated<mccmsg::CmdRespAnyPtr>();
        }
      , [this](group_stat, const mccmsg::Device& device, const GroupState& s)
        {
            bmcl::Option<mccmsg::Device> leader;
            if (s.leaderId.isSome())
            {
                leader = getDevice(s.leaderId.unwrap());
                assert(leader.isSome());
            }

            GroupPtr group = getGroup(device, s.term, mccmsg::idToGroup(s.groupId), leader);

            auto j = _devices.find(device);
            assert(j != _devices.end());
            if (j != _devices.end())
                j->second->update_my_group(group);

            if (!group->update(device, s.term, leader))
                return;

            mccmsg::Devices members;
            std::vector<mccmsg::DeviceId> unknown;
            members.reserve(s.memberIds.size());
            for (const auto& i : s.memberIds)
            {
                auto opt = getDevice(i);
                if (opt.isSome())
                    members.push_back(opt.unwrap());
                else
                    unknown.push_back(i);
            }

            send(_core, mccmsg::makeTm(new mccmsg::TmGroupState(device, mccmsg::GroupState(group->_group, leader, std::move(members), std::move(unknown)))));
        }
      , [this](group_stat_dev, const mccmsg::Device& device, const GroupDeviceState& s)
        {
            auto j = _devices.find(device);
            assert(j != _devices.end());
            if (s.groupId.isNone())
            {
                if (j != _devices.end())
                    j->second->update_my_group(bmcl::None);
                return;
            }

            bmcl::Option<mccmsg::Device> leader;
            if (s.leaderId.isSome())
            {
                leader = getDevice(s.leaderId.unwrap());
                assert(leader.isSome());
            }

            GroupPtr group = getGroup(device, s.term, mccmsg::idToGroup(s.groupId.unwrap()), leader);

            if (s.leaderId.isNone())
                return;

            if (!group->update(device, s.term, leader))
                return;
        }
      , [this](mccnet::group_dev, const mccmsg::ProtocolId& id, const caf::actor& a)
        {
            if (!a)
                return;
            monitor(a);
            _devices.emplace(id.device(), new Dev(id, a, bmcl::None, 0));

            for (const auto& i : _devices)
            {
                if (id.device() == i.first)
                    continue;
                //auto d = i.first;
            }
        }
      , [this](timerAtom)
        {
            for (Groups::iterator i = _groups.begin(); i != _groups.end();)
            {
                if (i->second->_confirmed_devices.empty())
                    i = _groups.erase(i);
                else
                    ++i;
            }
            delayed_send(this, std::chrono::seconds(10), timerAtom::value);
        }
    };
}

void GroupHandler::deleteDevice(const mccmsg::Device& d)
{
    auto i = std::find_if(_devices.begin(), _devices.end(), [&d](const typename Devs::value_type& i) { return i.second->id.device() == d; });
    if (i != _devices.end())
        _devices.erase(i);

    auto j = std::find_if(_groups.begin(), _groups.end(), [&d](const typename Groups::value_type& i) { return i.second->_leader == d; });
    if (j != _groups.end())
        _groups.erase(j);
}

bmcl::Option<mccmsg::Device> GroupHandler::getDevice(bmcl::Option<mccmsg::DeviceId> id) const
{
    if (id.isNone())
        return bmcl::None;
    auto i = std::find_if(_devices.begin(), _devices.end(), [id](const typename Devs::value_type& d) { return d.second->id.id() == id; });
    if (i != _devices.end())
        return i->first;
    //assert(false);
    return bmcl::None;
}

bmcl::Result<DevPtr, caf::error> GroupHandler::getLeader(const bmcl::Option<mccmsg::Group>& group)
{
    if (group.isNone())
        return mccmsg::make_error(mccmsg::Error::GroupNotSet);

    const auto& i = _groups.find(group.unwrap());
    if (i == _groups.end())
        return mccmsg::make_error(mccmsg::Error::GroupUnknown);

    if (i->second->_leader.isNone())
        mccmsg::make_error(mccmsg::Error::GroupWithoutLeader);

    const auto& j = _devices.find(i->second->_leader.unwrap());
    if (j == _devices.end())
    {
        assert(false);
        return mccmsg::make_error(mccmsg::Error::GroupLeaderUnknown);
    }

    return j->second;
}

void GroupHandler::execute(CmdPtr&& cmd, const mccmsg::CmdGroupNew& msg)
{
    std::vector<mccmsg::DeviceId> ids;
    for (const auto& i : msg.members())
    {
        const auto& j = _devices.find(i);
        if (j != _devices.end())
            ids.push_back(j->second->id.id());
        else
        {
            BMCL_WARNING() << "Неизвестное устройство: " << i.toStdString();
        }
    }

    for (const auto& i : msg.members())
    {
        const auto& j = _devices.find(i);
        if (j == _devices.end())
            continue;
        auto d = j->second->id;
        mccmsg::Group group = msg.group().unwrap();
        request(j->second->a, caf::infinite, group_cmd_new::value, group, ids).then
        (
            [this, cmd](caf::unit_t)
            {
                cmd->sendDone();
            }
          , [this, d, group](const caf::error& e)
            {
                BMCL_WARNING() << fmt::format("Ошибка создания группы {} устройтсвом {}: {}", mccmsg::groupToId(group), d.device().toStdString(), system().render(e));
            }
        );
    }
}

void GroupHandler::execute(CmdPtr&& cmd, const mccmsg::CmdGroupDelete& msg)
{
    if (msg.group().isNone())
    {
        cmd->sendFailed(mccmsg::Error::GroupNotSet);
        return;
    }

    mccmsg::Group group = msg.group().unwrap();

    for (const auto& i : _devices)
    {
        bmcl::OptionRc<Group> g = i.second->_group;
        if (g.isNone() || g.unwrap()->_group != group)
            continue;
        mccmsg::Device d = i.second->id.device();
        request(i.second->a, caf::infinite, group_cmd_del::value, group).then
        (
            [this, cmd](caf::unit_t) { cmd->sendDone(); }
          , [this, d, group](const caf::error& e)
            {
                BMCL_WARNING() << fmt::format("Ошибка отправки команды на удаление группы {} устройтсвом {}: {}", mccmsg::groupToId(group), d.toStdString(), system().render(e));
            }
        );
    }
}

void GroupHandler::execute(CmdPtr&& cmd, const mccmsg::CmdGroupAttach& msg)
{
    auto r = getLeader(msg.group());
    if (r.isErr())
    {
        cmd->sendFailed(r.takeErr());
        return;
    }
    const Dev& leader = *r.unwrap();

    const auto& d = _devices.find(msg.attachable());
    if (d == _devices.end())
    {
        assert(false);
        cmd->sendFailed(mccmsg::Error::DeviceUnknown);
        return;
    }

    request(leader.a, caf::infinite, group_cmd_att::value, msg.group().unwrap(),  d->second->id.id()).then
    (
        [cmd](caf::unit_t)
        {
            cmd->sendDone();
        }
      , [this, cmd](const caf::error& e)
        {
            cmd->sendFailed(mccmsg::make_error(mccmsg::Error::CmdFailed, system().render(e)));
        }
    );
}

void GroupHandler::execute(CmdPtr&& cmd, const mccmsg::CmdGroupDetach& msg)
{
    auto r = getLeader(msg.group());
    if (r.isErr())
    {
        cmd->sendFailed(r.takeErr());
        return;
    }
    const Dev& leader = *r.unwrap();

    const auto& d = _devices.find(msg.device().unwrap());
    if (d == _devices.end())
    {
        assert(false);
        cmd->sendFailed(mccmsg::Error::DeviceUnknown);
        return;
    }

    request(leader.a, caf::infinite, group_cmd_det::value, msg.group().unwrap(), d->second->id.id()).then
    (
        [cmd](caf::unit_t)
        {
            cmd->sendDone();
        }
      , [this, cmd](const caf::error& e)
        {
            cmd->sendFailed(mccmsg::make_error(mccmsg::Error::CmdFailed, system().render(e)));
        }
    );
}

void GroupHandler::execute(CmdPtr&& cmd, const mccmsg::CmdGroupSwitch&)
{
    cmd->sendFailed(mccmsg::Error::NotImplemented);
}

void GroupHandler::execute(CmdPtr&& cmd)
{
    const mccmsg::DevReq& msg = *cmd->item();
    auto r = getLeader(msg.group());
    if (r.isErr())
    {
        cmd->sendFailed(r.takeErr());
        return;
    }
    const Dev& leader = *r.unwrap();

    request(leader.a, caf::infinite, group_cmd::value, msg.group().unwrap(), cmd->item()).then
    (
        [cmd](caf::unit_t)
        {
            cmd->sendDone();
        }
      , [this, cmd](caf::error& e)
        {
            cmd->sendFailed(std::move(e));
        }
    );
}

caf::actor createGroupActor(caf::event_based_actor* spawer, const caf::actor& core)
{
    return spawer->spawn<GroupHandler>(core, "net.group");
}
}
