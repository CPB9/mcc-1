#pragma once

#include "mcc/vis/Config.h"
#include "mcc/vis/RadarParams.h"
#include "mcc/vis/Region.h"
#include "mcc/vis/Radar.h"
#include "mcc/plugin/PluginData.h"

#include <bmcl/Option.h>

#include <QObject>

#include <unordered_map>

namespace mccvis {

class RadarGroup;

typedef std::shared_ptr<RadarGroup> RadarGroupPtr;

class MCC_VIS_DECLSPEC RadarGroup : public QObject {
    Q_OBJECT
public:
    explicit RadarGroup(QObject* parent = nullptr);

   // const RadarPtr& add(const mccgeo::LatLon& pos, const RadarParams& params, const QString& name);
    void add(const RadarPtr& radar);

    void notifyUpdate(const RadarPtr& radar);
    void remove(const RadarPtr& radar);
    std::size_t size() const;
    const std::vector<RadarPtr>& radars() const;

    static RadarGroupPtr deserialize(const QByteArray& data);
    QByteArray serialize() const;

signals:
    void radarAdded(const RadarPtr& radar);
    void radarUpdated(const RadarPtr& radar);
    void radarRemoved(const RadarPtr& radar);
    void radarsReset();

public:
    void addNoEmit(const mccgeo::LatLon& pos, const ViewParams& params);

    std::vector<RadarPtr> _radars;
};

class MCC_VIS_DECLSPEC RadarControllerPluginData : public mccplugin::PluginData {
public:
    static constexpr const char* id = "mcc::WidgetPlugin";

    RadarControllerPluginData(const RadarGroupPtr& controller);
    ~RadarControllerPluginData();

    const RadarGroupPtr& radarController() const;

private:
    RadarGroupPtr _controller;
};
}
