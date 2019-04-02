#include "mcc/vis/RadarGroup.h"

#include <bmcl/MemWriter.h>
#include <bmcl/Buffer.h>
#include <bmcl/MemReader.h>
#include <bmcl/Alloca.h>
#include <bmcl/Result.h>

#include <QFile>

namespace mccvis {

RadarGroup::RadarGroup(QObject* parent)
    : QObject(parent)
{
}

void RadarGroup::addNoEmit(const mccgeo::LatLon& pos, const ViewParams& params)
{
    _radars.emplace_back(new Radar(pos, params));
}

// const RadarPtr& RadarGroup::add(const mccgeo::LatLon& pos, const RadarParams& params, const QString& name)
// {
//     addNoEmit(pos, params, name);
//     emit radarAdded(_radars.back());
//     return _radars.back();
// }

void RadarGroup::add(const RadarPtr& radar)
{
    _radars.push_back(radar);
    emit radarAdded(radar);
}

void RadarGroup::remove(const RadarPtr& radar)
{
    for (auto it = _radars.begin(); it < _radars.end(); it++) {
        if (*it == radar) {
            RadarPtr r = *it;
            _radars.erase(it);
            emit radarRemoved(r);
            return;
        }
    }
}

void RadarGroup::notifyUpdate(const RadarPtr& radar)
{
    emit radarUpdated(radar);
}

static void serializeRadar(const Radar* radar, QByteArray* dest)
{
    bmcl::Buffer w;
    radar->serialize(&w);
    dest->append((char*)w.data(), (int)w.size());
}

QByteArray RadarGroup::serialize() const
{
    QByteArray data;
    for (const RadarPtr& radar : _radars) {
        serializeRadar(radar.get(), &data);
    }
    return data;
}

RadarGroupPtr RadarGroup::deserialize(const QByteArray& data)
{
    bmcl::MemReader r(data.constData(), data.size());
    RadarGroupPtr group = std::make_shared<RadarGroup>();

    while (!r.isEmpty()) {
        auto rv = Radar::deserialize(&r);
        if (rv.isErr()) {
            return group;
        }
        RadarPtr radar = rv.take();
        radar->setSavePath(bmcl::None);
        group->_radars.push_back(radar);
    }
    return group;
}

std::size_t RadarGroup::size() const
{
    return _radars.size();
}

const std::vector<RadarPtr>& RadarGroup::radars() const
{
    return _radars;
}

RadarControllerPluginData::RadarControllerPluginData(const RadarGroupPtr& controller)
    : mccplugin::PluginData(RadarControllerPluginData::id)
    , _controller(controller)
{
}

RadarControllerPluginData::~RadarControllerPluginData()
{
}

const RadarGroupPtr& RadarControllerPluginData::radarController() const
{
    return _controller;
}
}
