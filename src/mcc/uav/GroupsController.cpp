#include "mcc/uav/GroupsController.h"
#include "mcc/uav/UavController.h"
#include "mcc/uav/Group.h"
#include "mcc/uav/Uav.h"
#include "mcc/uav/ExchangeService.h"
#include "mcc/msg/Objects.h"
#include "mcc/msg/exts/Position.h"

#include <algorithm>

namespace mccuav {

GroupsController::GroupsController(mccuav::UavController* uavController,
                                   mccuav::ExchangeService* service,
                                   QObject* parent)
    : QObjectRefCountable<QObject>(parent)
    , _groups()
    , _uavController(uavController)
    , _exchangeService(service)
{

    connect(_exchangeService.get(), &mccuav::ExchangeService::traitGroupState, this, &GroupsController::handleGroupState);

    connect(uavController, &UavController::uavStateChanged, this, &GroupsController::handleUavState);
    connect(uavController, &UavController::uavTmStorageUpdated, this, &GroupsController::handleUavStorage);
}

GroupsController::~GroupsController()
{
    _posHandlers.clear();
    for (auto group : _groups)
    {
        delete group;
    }
}

bmcl::Option<const mccmsg::Devices&> GroupsController::groupUavs(const mccmsg::Group& groupId) const
{
    const auto it = std::find_if(_groups.begin(), _groups.end(), [&groupId](const Group* group){return group->id() == groupId;});
    if(it == _groups.end())
        return bmcl::None;
    return (*it)->uavs();
}

const Group* GroupsController::uavGroup(const mccmsg::Device& deviceId) const
{
    for(auto group : _groups)
    {
        if(group->hasUav(deviceId))
        {
            return group;
        }
    }

    return nullptr;
}

const Group* GroupsController::uavGroup(const Uav* uav) const
{
    for(auto group : _groups)
    {
        if(group->hasUav(uav))
        {
            return group;
        }
    }
    return nullptr;
}

const Group* GroupsController::group(const mccmsg::Group& groupId) const
{
    const auto it = std::find_if(_groups.begin(), _groups.end(), [&groupId](const Group* group){return group->id() == groupId;});
    if(it == _groups.end())
        return nullptr;
    return *it;
}

const Group*GroupsController::group(const mccmsg::DeviceId& groupNumber) const
{
    const auto it = std::find_if(_groups.begin(), _groups.end(), [groupNumber](const Group* group){return mccmsg::groupToId(group->id()) == groupNumber;});
    if(it == _groups.end())
        return nullptr;
    return *it;
}

const Group* GroupsController::selectedGroup() const
{
    mccuav::UavController* dm = _uavController.get();
    if(dm == nullptr || dm->selectedUav() == nullptr)
        return nullptr;

    return uavGroup(dm->selectedUav());
}

std::vector<const Group*> GroupsController::groups() const
{
    std::vector<const Group*> constGroups;
    constGroups.reserve(_groups.size());
    for(auto group : _groups)
    {
       constGroups.push_back(group);
    }

    return constGroups;
}

size_t GroupsController::groupsCount() const
{
    return _groups.size();
}

size_t GroupsController::groupUavsCount(const mccmsg::Group& groupId) const
{
    const auto it = std::find_if(_groups.begin(), _groups.end(), [&groupId](const Group* group){return group->id() == groupId;});
    if(it == _groups.end())
        return 0;
    return (*it)->size();
}

std::vector<Uav*> GroupsController::groupUavs(const Group* group) const
{
    std::vector<Uav*> devices;

    mccuav::UavController* dm = _uavController.get();
    if(dm == nullptr || group == nullptr)
        return devices;

    devices.reserve(group->size());
    for(auto devId : group->uavs())
    {
        auto deviceOpt = dm->uav(devId);
        if(deviceOpt.isSome())
            devices.push_back(deviceOpt.unwrap());
    }

    return devices;
}

Uav* GroupsController::groupLeader(const Group* group) const
{
    mccuav::UavController* dm = _uavController.get();
    if(dm == nullptr || group == nullptr || group->leader().isNone())
        return nullptr;

    auto devOpt = dm->uav(group->leader().unwrap());
    return devOpt.isSome() ? devOpt.unwrap() : nullptr;
}

bool GroupsController::isLeader(const Uav* device) const
{
    if(device == nullptr)
        return false;

    const Group* group = uavGroup(device);
    if(group != nullptr)
    {
        return group->leader() == device->device();
    }

    return false;
}

void GroupsController::handleGroupState(const mccmsg::TmGroupStatePtr& state)
{
    // Find or create group
    const auto it = std::find_if(_groups.begin(), _groups.end(), [&state](const Group* group){return group->id() == state->state().group();});
    if(it == _groups.end())
    {
        Group* group = new Group(state);
        if(group->state().group().isNil())
            BMCL_WARNING() << "Group - empty group uuid";

        _groups.push_back(group);

        emit groupAdded(group->id());
    }
    else
    {
        if((*it)->updateState(state))
        {
            emit groupChanged((*it)->id());
        }
    }
}

void GroupsController::handleUavState(Uav* uav)
{
    if(uav == nullptr)
        return;

    auto i = std::find_if(_groups.begin(), _groups.end(), [uav](Group* g) { return g->hasUav(uav); });
    if (i == _groups.end())
        return;

    // Only removing empty groups. Other info is from GroupState
    Group* oldGroup = *i;
    assert(oldGroup);
    if (oldGroup == nullptr)
        return;

    bmcl::Option<mccmsg::Group> curGroupOpt;
    if (!uav->tmStorage().isNull())
        curGroupOpt = uav->tmStorage()->group();
    // was changed
    if(curGroupOpt != oldGroup->id())
    {
        oldGroup->removeUav(uav);
        emit groupChanged(oldGroup->id());

        removeEmptyGroups();
    }
}

void GroupsController::handleUavPosition(const mccmsg::Device& device, const mccmsg::TmPosition* pos)
{
    UavController* manager = _uavController.get();
    for (auto group : _groups)
    {
        if (!group->hasUav(device))
            continue;

        if (group->geometry().isSome() && bmcl::toMsecs(bmcl::SystemClock::now() - group->geometry()->time()).count() < 200)
            continue;

        double dt = 0.0;
        if (group->geometry().isSome())
            dt = bmcl::toMsecs(group->geometry()->time() - pos->updated()).count() / 1000.0;

        std::vector<mccgeo::Position> positions;
        positions.reserve(group->size());
        for (auto devId : group->uavs())
        {
            auto deviceOpt = manager->uav(devId);
            if (deviceOpt.isSome())
            {
                Uav* device = deviceOpt.unwrap();

                if (device->position().isNone())
                    continue;

                if (device->isAlive() && device->isActivated())
                    positions.push_back(device->position().unwrap());
            }
        }

        group->updateGeometry(_enuConverter.calcGroupParams(positions, dt));

        emit groupGeometryChanged(group->id());
    }
}

void GroupsController::handleUavStorage(mccuav::Uav* uav)
{
    auto st = uav->tmStorage();
    auto ext = st->getExtension<mccmsg::TmPosition>();
    if (ext.isNone())
        return;

    bmcl::Rc<mccmsg::TmPosition> pos = ext.unwrap();
    _posHandlers[uav->device()] = pos->addHandler([this, d = uav->device(), pos](){ handleUavPosition(d, pos.get()); }, true);
}

void GroupsController::removeEmptyGroups()
{
    for (auto it = _groups.begin(); it != _groups.end();)
    {
        if ((*it)->empty())
        {
            Group* removingGroup = *it;
            mccmsg::Group id = removingGroup->id();

            it = _groups.erase(it);
            emit groupRemoved(id);

            delete removingGroup;
            emit groupRemovingCompleted(id);
        }
        else
            ++it;
    }
}

GroupsControllerPluginData::GroupsControllerPluginData(GroupsController* groupsController)
    : mccplugin::PluginData(id)
    , _groupsController(groupsController)
{
}

GroupsControllerPluginData::~GroupsControllerPluginData()
{
}

GroupsController* GroupsControllerPluginData::groupsController()
{
    return _groupsController.get();
}

const GroupsController* GroupsControllerPluginData::groupsController() const
{
    return _groupsController.get();
}
}
