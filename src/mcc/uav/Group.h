#pragma once
#include <QtGlobal>
#include <QMetaType>
#include "mcc/msg/FwdExt.h"
#include "mcc/msg/GroupState.h"
#include "mcc/msg/Objects.h"
#include "mcc/geo/GroupGeometry.h"

namespace mccuav {

class Uav;

class MCC_UAV_DECLSPEC Group
{
public:
    explicit Group(const mccmsg::TmGroupStatePtr& state);

    bool updateState(const mccmsg::TmGroupStatePtr& state);
    void updateGeometry(const mccgeo::GroupGeometry& geometry);

    const mccmsg::Group& id() const {return _state.group();}
    mccmsg::GroupId number() const;
    const bmcl::Option<mccmsg::Device>& leader() const {return _state.leader();}
    const std::vector<mccmsg::Device>& uavs() const {return _state.members();}
    const std::vector<mccmsg::DeviceId>& unknownUavs() const {return _state.unknown_members();}
    const mccmsg::GroupState& state() const {return _state;}

    const bmcl::Option<mccgeo::GroupGeometry>& geometry() const { return _geometry; }

    bool isEmpty() const;
    size_t size() const;
    bool empty() const {return _state.members().empty();}
    bool hasUav(const mccmsg::Device& deviceId) const;
    bool hasUav(const mccuav::Uav* uav) const;

    void removeUav(const mccmsg::Device& deviceId);
    void removeUav(const mccuav::Uav* uav);

    bool operator==(const mccmsg::Group& id) {return _state.group() == id;}
    bool operator!=(const mccmsg::Group& id) {return _state.group() != id;}

    inline friend bool operator< (const Group &l, const Group &r) { return l.id() < r.id(); }
    inline friend bool operator==(const Group &l, const Group &r) { return l.id() == r.id(); }
    inline friend bool operator!=(const Group &l, const Group &r) { return l.id() != r.id(); }

private:
    mccmsg::GroupState                  _state;
    bmcl::Option<mccgeo::GroupGeometry> _geometry;

    Q_DISABLE_COPY(Group)
};

}

Q_DECLARE_METATYPE(const mccuav::Group*);
