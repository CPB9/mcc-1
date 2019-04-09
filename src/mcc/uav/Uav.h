#pragma once

#include <QObject>
#include <QUuid>
#include <QPixmap>
#include <QTime>
#include <QMap>
#include <QMetaType>

#include "mcc/Config.h"
#include "mcc/uav/Rc.h"
#include "mcc/uav/Structs.h"
#include "mcc/uav/PointOfInterest.h"
#include "mcc/uav/UserParam.h"
#include "mcc/uav/UavIconGenerator.h"

#include "mcc/ui/Fwd.h"

#include "mcc/msg/FwdExt.h"
#include "mcc/msg/Stats.h"
#include "mcc/msg/obj/Firmware.h"
#include "mcc/msg/obj/Device.h"
#include "mcc/msg/ptr/Fwd.h"
#include "mcc/msg/Route.h"

#include "mcc/geo/Attitude.h"

#include <bmcl/OptionPtr.h>

namespace mccuav {

class Route;
class Settings;
class UavController;
class UavExecCommands;
class MotionExtension;
class UavErrors;
class UavErrorsFilteredModel;

struct RouteSettings
{
    QColor _color;
};

struct UavSettingsMap
{
    std::map<mccmsg::RouteName, RouteSettings> _routes;
};

enum class UavFeatures
{
    RouteCanActivated,
    RouteCanAdded,
};

class MCC_UAV_DECLSPEC Uav : public QObject
{
    Q_OBJECT

public:
    ~Uav() override;

    const mccmsg::Device& device() const;
    const mccmsg::Protocol& protocol() const;
    mccmsg::DeviceId deviceId() const;

    const QString& getName() const;
    const QString& getInfo() const;

    //const QString&    name()          const;
    const QPixmap&    pixmap()        const;
    const QPixmap&    previewPixmap() const;
    QPixmap           pixmap(float scale) const;
    const UavIconGenerator&  pixmapGenerator() const;
    const QColor&     color()         const;
    Route*            activeRoute();
    Route*            selectedRoute();
    void              setSelectedRoute(int routeId);
    void              selectRoute(Route* route);
    bmcl::Option<std::size_t>   nextWaypoint()  const;
    void              setSourcePixmapFile(bmcl::Bytes pixmap);
    void              setColor(const QColor& color, double scale);
    void              setColor(const QColor& color);
    QTime             lastTmMsgTime()   const;
    const QDateTime&  lastTmMsgDateTime()   const;
    bool              isAlive()         const;
    void              setAlive(bool active);
    void              setSignalGood();
    void              setSignalBad();

    bool              isStateActive();
    bool              isActivated() const;

    const TrackSettings& trackSettings() const;

    const mccmsg::StatDevice& statDevice() const;
    void setStatDevice(const mccmsg::StatDevice& state);

    const mccmsg::Channels& channels() const;
    void setChannels(const mccmsg::Channels& channels);

    const mccmsg::DeviceDescription& deviceDescription() const;
    void setDeviceDescription(const mccmsg::DeviceDescription& deviceDescription);

    const bmcl::OptionRc<const mccmsg::FirmwareDescriptionObj>& firmwareDescription() const;
    void setFirmwareDescription(const mccmsg::FirmwareDescription& frm);

    const QVector<Route*>& routes() const;

    void addRoute(Route* route);
    Route* findRouteById(int id) const;
    bmcl::OptionPtr<Route> findRoute(mccmsg::RouteName name) const;

    void setActiveRoute(Route* route);
    void setActiveRouteAndPoint(Route* route, int index);
    void setEmptyActiveRoute();
    void changeRouteDirection(Route* route, bool forward);

    void setNextWaypoint(Route* route, int index);
    void clearNextWaypoint(Route* route);

    void setMotionLimits(const UavMotionLimits& limits);
    const UavMotionLimits& motionLimits() const;

    void createRoute(mccmsg::RouteName name);
    void removeRoute(mccmsg::RouteName name);

    bmcl::Option<mccmsg::Group> group() const;
    bmcl::Option<mccmsg::GroupId> groupId() const;
    int qmlGroupId() const;

    const PointOfInterestPtrs& pointsOfInterest() const;
    void addPointOfInterest(const PointOfInterestPtr& point);
    void removePointOfInterest(const PointOfInterestPtr& point);
    void clearPointsOfInterest();

    const UserParamPtrs& userParams() const;
    void addUserParam(const UserParamPtr& point);
    void removeUserParam(const UserParamPtr& point);
    void clearUserParams();

    double distanceToWaypoint(const mccmsg::Waypoint& wp, double* direction = nullptr);
    mccuav::MotionExtension* motionExtension();

    QString getDeviceDescriptionText() const;

    bool hasFeature(UavFeatures feature) const;

    void onCmdAdded(const mccmsg::DevReqPtr& cmd);
    void onCmdStateChanged(const mccmsg::DevReqPtr& req, const mccmsg::Request_StatePtr& state);
    void onCmdRemoved(const mccmsg::DevReqPtr& cmd);

    UavExecCommands* execCommands() const;
    UavErrors* errors();
    UavErrorsFilteredModel* filteredErrors();
    const UavErrorsFilteredModel* filteredErrors() const;

    const bmcl::Rc<mccmsg::ITmStorage>& tmStorage() const;

    bool isRegistered() const;
    const bmcl::Option<const mccgeo::Position&> position() const;
    const bmcl::Option<const mccgeo::Attitude&> attitude() const;
    const bmcl::Option<double> speed() const;
    const bmcl::Option<const mccgeo::Position&> velocity() const;
signals:
    void activatedChanged();
    void groupIdChanged();
    void motionLimitsChanged();
    void trackSettingsChanged();
    void clearTrackRequest();

    void pixmapChanged       (const QPixmap& pixmap);
    void nextWaypointChanged (int index);
    void targetHeadingChanged(double heading);

    void routeUploaded();

    void uavStatisticsChanged();
    void channelsChanged();
    void signalGood();
    void signalBad();

    void routeAdded(Route* route);
    void routeRemoved(Route* route);
    void routeSelectionChanged(Route* route);

    void activeRouteChanged(Route* route);
    void deviceDescriptionUpdated(const mccmsg::DeviceDescription& deviceDescription);

    void pointOfInterestAdded(const PointOfInterestPtr& poi);
    void pointOfInterestRemoved(const PointOfInterestPtr& poi);

    void userParamAdded(const UserParamPtr& poi);
    void userParamRemoved(const UserParamPtr& poi);
    
    void protocolIdUpdated();
    void newTmParamAvailable(const std::string& param);

    void tmStorageUpdated();
    void tmViewUpdated(const bmcl::Rc<const mccmsg::ITmViewUpdate>& update);
    void showAlert(const QString& message = QString());

    void positionChanged();
    void orientationChanged();
public slots:
    void processRouteState(const mccmsg::TmRoutePtr& route);
    void processRoutesList(const mccmsg::TmRoutesListPtr& routesList);
    void processActivated(bool activated);
    void processSetTmView(const bmcl::Rc<const mccmsg::ITmView>& view);
    void processUpdateTmStatusView(const bmcl::Rc<const mccmsg::ITmViewUpdate>& update);

    void setLastMsgTime(const QDateTime& time);

    void resetEditableRoute(int routeIdx = 0);
    void resetEditableRoute(Route* route);
    void uploadEditableRoute();
    void saveSettings();
    void setTrackSettings(mccuav::TrackMode mode, const bmcl::Option<int>& seconds = bmcl::None, const bmcl::Option<int>& meters = bmcl::None);

protected:
    friend class UavController;
    Uav(UavController* manager, const mccmsg::DeviceDescription& id);

    mccmsg::Channels    _channels;

    UavIconGenerator    _iconGenerator;
    QPixmap             _pixmap;
    QPixmap             _previewPixmap;
    QString             _sourcePixmap;
    QColor              _color;
    QString             _uiFile;

    UavController*      _manager; //non owning ref
    QDateTime           _lastTmMsgDateTime;
    size_t              _rcvdPackets;

    bool                _isAlive;
    bool                _isActivated;

    bmcl::Option<mccmsg::RouteName> _activeRouteName;
    int                 _selectedRouteId;

    QVector<Route*>     _routes;
    UavMotionLimits     _motionLimits;
    double              _targetHeading;

    mccuav::MotionExtension* _motionExtension;

    TrackSettings       _trackSettings;

    mccmsg::StatDevice  _statistics;

    mccmsg::DeviceDescription                   _deviceDescription;
    bmcl::OptionRc<const mccmsg::FirmwareDescriptionObj> _frmDescription;

    PointOfInterestPtrs                         _pointsOfInterest;
    UserParamPtrs _userParams;
    UavSettingsMap      _settings;

    UavExecCommands*    _execCommands;
    UavErrors*          _errors;
    UavErrorsFilteredModel* _filteredErrors;

    bmcl::Rc<mccmsg::ITmStorage> _tmStorage;

    QString _deviceNameCached;
    QString _deviceInfoCached;
};
}

Q_DECLARE_METATYPE(mccuav::Uav*);
Q_DECLARE_METATYPE(const mccuav::Uav*);
