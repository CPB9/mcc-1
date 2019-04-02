#pragma once

#include "mcc/vis/Config.h"
#include "mcc/vis/Rc.h"
#include "mcc/vis/RadarParams.h"
#include "mcc/vis/Region.h"
#include "mcc/hm/HmReader.h"
#include "mcc/geo/Geod.h"

#include <bmcl/Fwd.h>
#include <bmcl/Option.h>

#include <QString>
#include <QObject>

namespace mccvis {

class Radar;

typedef Rc<Radar> RadarPtr;

class MCC_VIS_DECLSPEC Radar : public RefCountable {
public:
    Radar();
    ~Radar();
    Radar(const mccgeo::LatLon& position);
    Radar(const mccgeo::LatLon& position, const ViewParams& params);

    void setVisibility(bool isVisible);
    void setParams(const ViewParams& params);
    void setPosition(const mccgeo::LatLon& position);
    bool saveAs(const QString& path);
    bool save();
    void setSavePath(const bmcl::Option<QString>& path);
    void setModified(bool isModified);

    void serialize(bmcl::Buffer* dest) const;
    static bmcl::Result<Rc<Radar>, void> deserialize(bmcl::MemReader* src);
    static bmcl::Result<Rc<Radar>, void> loadFrom(const QString& path);

    bool isVisible() const;
    const ViewParams& viewParams() const;
    const QString& name() const;
    const mccgeo::LatLon& position() const;
    const bmcl::Option<QString>& savePath() const;
    bool isModified() const;

    std::vector<Point> visionArea(const mcchm::HmReader* handler, const mccgeo::Geod& geod, const std::vector<mccgeo::PositionAndDistance>& profile) const;

private:
    void calcVisionRadius();
    mccgeo::LatLon _position;
    ViewParams _params;
    QString _name;
    bmcl::Option<QString> _savePath;
    bool _isVisible;
    bool _isModified;
};
}
