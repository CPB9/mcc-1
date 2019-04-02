#pragma once

#include "mcc/msg/FwdExt.h"
#include "mcc/msg/Nav.h"
#include "mcc/msg/obj/Device.h"
#include "mcc/geo/EnuPositionHandler.h"
#include "mcc/plugin/PluginData.h"
#include "mcc/uav/Rc.h"
#include "mcc/ui/QObjectRefCountable.h"

#include <QObject>
#include <vector>
#include <bmcl/Option.h>

namespace mccuav {

class ExchangeService;
class Group;
class Uav;
class UavController;

class MCC_UAV_DECLSPEC GroupsController : public mccui::QObjectRefCountable<QObject>
{
    Q_OBJECT

public:
    GroupsController(mccuav::UavController* uavController,
                     mccuav::ExchangeService* service,
                     QObject *parent = nullptr);
    ~GroupsController() override;

    const Group* uavGroup(const mccmsg::Device& deviceId) const;
    const Group* uavGroup(const mccuav::Uav* uav) const;
    const Group* group(const mccmsg::Group& groupId) const;
    const Group* group(const mccmsg::DeviceId& groupNumber) const;
    const Group* selectedGroup() const;
    std::vector<const Group*> groups() const; //TODO: заменить на range
    bmcl::Option<const mccmsg::Devices&> groupUavs(const mccmsg::Group& groupId) const;

    size_t groupsCount() const;
    size_t groupUavsCount(const mccmsg::Group& groupId) const;

    std::vector<Uav*> groupUavs(const Group* group) const;
    Uav* groupLeader(const Group* group) const;
    bool isLeader(const mccuav::Uav* device) const;

public slots:
    void handleGroupState(const mccmsg::TmGroupStatePtr& state);
    void handleUavState(mccuav::Uav* uav);
    void handleNavigationMotion(const mccmsg::TmMotionPtr& motion);

signals:
    void groupAdded(const mccmsg::Group& groupId);
    void groupChanged(const mccmsg::Group& groupId);
    void groupRemoved(const mccmsg::Group& groupId);
    void groupRemovingCompleted(const mccmsg::Group& groupId);

    void groupGeometryChanged(const mccmsg::Group& groupId);

private:
    void removeEmptyGroups();

    std::vector<Group*>         _groups;
    mccgeo::EnuPositionHandler _enuConverter;
    Rc<mccuav::UavController> _uavController;
    Rc<mccuav::ExchangeService> _exchangeService;

    Q_DISABLE_COPY(GroupsController)
};

class MCC_UAV_DECLSPEC GroupsControllerPluginData : public mccplugin::PluginData {
public:
    static constexpr const char* id = "mcc::GroupsControllerPluginData";

    GroupsControllerPluginData(GroupsController* groupsController);
    ~GroupsControllerPluginData();

    GroupsController* groupsController();
    const GroupsController* groupsController() const;

private:
    Rc<GroupsController> _groupsController;
};
}
