#pragma once

#include <QObject>
#include <QVector>
#include <QPen>

#include "mcc/uav/Fwd.h"
#include "mcc/uav/Structs.h"
#include "mcc/uav/RouteStyle.h"

#include <bmcl/Option.h>

#include "mcc/msg/Route.h"
#include "mcc/geo/Fwd.h"

Q_DECLARE_METATYPE(mccmsg::Waypoint)

namespace mccuav {

class MCC_UAV_DECLSPEC Route : public QObject
{
    Q_OBJECT

public:
    enum class FileFormat
    {
        Json,
        KmlLineString
    };

    enum class EditMode
    {
        Latitude,
        Longitude,
        Altitude,
        Speed,
        Flags
    };

    struct WaypointHint
    {
        size_t indexAfter;
        double distance;
        mccgeo::LatLon latLon;

        WaypointHint(size_t indexAfter, double distance, const mccgeo::LatLon& latLon)
            : indexAfter(indexAfter), distance(distance), latLon(latLon)
        {
        }
    };

    Route(mccuav::Uav* uav, const QString& name, int id, size_t maxCount, bool isBuffer = false);
    ~Route();

    void setMotionLimits(const UavMotionLimits& limits);

    int                    id() const;
    const mccmsg::Waypoint&  waypointAt(int idx) const;
    mccmsg::Waypoint&       waypointAt(int idx);
    const mccmsg::Waypoints& waypointsList() const;
    int                    waypointsCount() const {return _points.size();}
    bool                   isValid() const;
    bool                   isEmpty() const {return _points.empty();}
    bool                   isLoop() const;
    bool                   isUserVisible() const;
    bool                   isReadOnly() const;
    bool                   isInverted() const;
    bool                   showPointsOnly() const;
    bool                   showDetails() const;
    bmcl::Option<const mccmsg::Waypoint&> activePoint() const;
    bmcl::Option<std::size_t> activePointIndex() const;
    const std::vector<std::size_t>& selectedPointIndexes() const;
    bool                   isPointSelected(size_t index) const;
    bool                   isPointValid(size_t index) const;
    bool                   isWaypointVelocityValid(const mccmsg::Waypoint& wp) const;
    bool                   isWaypointAltitudeValid(const mccmsg::Waypoint& wp) const;
    size_t                 indexOf(const mccmsg::Waypoint& wp) const;
    bool                   isEnabled() const;
    bool                   isHidden() const;
    bmcl::Option<WaypointHint> waypointHint() const;

    const QPen&          pen() const;
    const QColor&        activePointColor() const;
    const QColor&        inactivePointColor() const;

    const QString&       name() const;

    mccgeo::Bbox computeBoundindBox() const;
    void copyFrom(Route* other);
    void update(const mccmsg::Route& other);

    Route*               buffer() const;
    void                 resetBuffer();
    void                 reverse();

    void                 save(const QString& path, FileFormat fmt) const;
    void                 load(const QString& path, const mccmsg::ProtocolController*);
    const RouteStylePtr& style() const;
    RouteState   state() const;

    QString generateSaveName() const;
    static constexpr const char* filenameExtension() {return ".route";}

public slots:
    void clear();
    bool addWaypoint(const mccmsg::Waypoint& waypoint);
    void addWaypointNoEmit(const mccmsg::Waypoint& waypoint);
    bool setWaypoint(const mccmsg::Waypoint& waypoint, int idx);
    void setWaipointAlt(double alt, int idx);
    bool insertWaypoint(const mccmsg::Waypoint& waypoint, int afterIndex = -1);
    bool canInsertWaypoint(int afterIndex = -1);
    bool removeWaypoint(int index);
    bool removeSelectedWaypoints();
    bool setActivePoint(const bmcl::Option<std::size_t>& index);

    void clearSelection();
    void selectAll();
    void deselectPoint(std::size_t index);
    void selectPoint(std::size_t index);
    void setSelectedPoint(std::size_t index, bool forced = false);
    void setSelectedPoints(const std::vector<std::size_t>& indexes, bool forced = false);
    void setInverted(bool inverse);
    void setShowPointsOnly(bool show);
    bool moveWaypointUp(int index);
    bool moveWaypointDown(int index);

    void setWaypoints(const mccmsg::Waypoints& waypoints);

    void setClosedPath(bool isRing);

    void setUserVisibility(bool isVisible);
    void setHidden(bool isHidden);
    void setReadOnly(bool isReadOnly);
    void setState(RouteState state);
    void setColor(const QColor& color);
    void setWaypointHint(const bmcl::Option<WaypointHint>& hint);

    void requestEditDialog(int index, EditMode mode);
    //void setStyle(const QPen& pen, const QColor& activePointColor, const QColor& inactivePointColor, bool showDetails);
signals:
    void allWaypointsChanged();
    void waypointOnlyAltChanged(const mccmsg::Waypoint& waypoint, int index);

    void waypointInserted(const mccmsg::Waypoint& waypoint, int index);
    void waypointRemoved(int index);
    void waypointMoved(int oldIndex, int newIndex);
    void waypointChanged(const mccmsg::Waypoint& waypoint, int index);

    void activeWaypointChanged(const bmcl::Option<std::size_t>& index);
    void selectedWaypointsChanged(std::size_t index, bool selected);

    void closedPathFlagChanged(bool isRing);
    void userVisibilityFlagChanged(bool isVisible);
    void readOnlyFlagChanged(bool readonly);
    bool inverseFlagChanged(bool inverse);
    void showPointsOnlyFlagChanged(bool show);
    void enabledFlagChanged(bool isEnabled);
    void hiddenFlagChanged(bool isHidden);
    void waypointHintChanged();

    void styleChagned(const RouteStylePtr& style);

    void showEditDialog(int index, EditMode mode);

private:
    void saveJson(const QString& path) const;
    void saveKmlLineString(const QString& path) const;

    void syncActivePoint();
    void syncSelectedPoint();
    void setDefaultProperties(mccmsg::Waypoint& wp);

    mccuav::Uav*                  _uav;
    mccmsg::Waypoints             _points;
    bool                          _isRing;
    bool                          _showDetails;
    bool                          _isSelected;
    int                           _id;
    int                           _maxCount;
    bool                          _isVisible;
    bool                          _isActive;
    bool                          _isReadOnly;
    bool                          _isInverse;
    bool                          _showPointsOnly;
    bmcl::Option<std::size_t>     _activePointIndex;
    bool                          _isEnabled;
    bool                          _isHidden;
    std::vector<std::size_t>      _selectedPointIndexes;
    bmcl::Option<WaypointHint>    _waypointHint;
    QString                       _name;
    Route*                        _buffer;
    RouteStylePtr                 _style;
    bmcl::Option<UavMotionLimits> _motionsLimits;
    Q_DISABLE_COPY(Route)
};
}
