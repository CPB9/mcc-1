#pragma once
#include <unordered_map>
#include <QObject>
#include <QVector>
#include <QMap>
#include <set>
#include <bmcl/Option.h>
#include <bmcl/OptionPtr.h>
#include <bmcl/StringView.h>

#include "mcc/uav/Uav.h"
#include "mcc/ui/Fwd.h"
#include "mcc/ui/QObjectRefCountable.h"
#include "mcc/uav/WaypointTemplateType.h"
#include "mcc/ui/UavSettings.h"
#include "mcc/hm/Fwd.h"

#include "mcc/plugin/PluginData.h"

#include "mcc/msg/Fwd.h"
#include "mcc/msg/File.h"
#include "mcc/msg/ptr/Tm.h"
#include "mcc/msg/obj/Firmware.h"

#include <bmcl/TimeUtils.h>

#include "mcc/geo/Vector3D.h"
#include "mcc/geo/EnuPositionHandler.h"
#include "mcc/geo/GroupGeometry.h"
#include "mcc/geo/Geod.h"
#include "mcc/uav/Rc.h"

class QMessageBox;
class QColor;

namespace photon { struct PacketResponse; }
namespace photon { class NodeView; }
namespace photon { class NodeViewUpdater; }

namespace mccmsg { class ProtocolController; }

Q_DECLARE_METATYPE(mccui::UavSettings);
Q_DECLARE_METATYPE(mccuav::UavController*);

namespace mccuav {

class ExchangeService;
class ChannelsController;
class GroupsController;
class RoutesController;

struct UiFileDescriptor
{
    mccmsg::File                file;
    mccmsg::Device              device;
    mccmsg::RequestId           cmdId;
    mccmsg::Request_StatePtr    cmdState;
    std::string                 path;

    UiFileDescriptor(const mccmsg::File& file, const mccmsg::Device& device, const std::string& path, mccmsg::RequestId cmdId)
        : file(file), device(device), cmdId(cmdId), path(path)
    {}
};

using OnError = std::function<void(const mccmsg::ErrorDscr&)>;
using OnSuccessCmd = std::function<void(const mccmsg::CmdRespAnyPtr&)>;
using OnState = std::function<void(const mccmsg::Request_StatePtr&)>;

class MCC_UAV_DECLSPEC ResponseHandle
{
public:
    ResponseHandle(mccuav::ExchangeService* service, mccmsg::DevReqPtr&& r);
    ResponseHandle(mccuav::ExchangeService* service, const mccmsg::DevReqPtr& r);
    ~ResponseHandle();
    mccmsg::DevReqPtr then(OnSuccessCmd&& f = nullptr, OnError&& e = nullptr, OnState&& s = nullptr);
private:
    bool _sent;
    mccuav::ExchangeService* _service; //FIXME: rc
    mccmsg::DevReqPtr _r;
};

class MCC_UAV_DECLSPEC UavController : public mccui::QObjectRefCountable<QObject>
{
    friend class ResponseHandle;
    Q_OBJECT
public:
    UavController(mccui::Settings* settings,
                  ChannelsController* chanController,
                  RoutesController* routesController,
                  const mccui::HeightmapController* hmController,
                  const mccmsg::ProtocolController* protocolController,
                  mccuav::ExchangeService* service);
    ~UavController() override;

    void init();

    QColor nextRouteColor();

    void unregisterUavAndChannel(Uav* uav);
    void unregisterUav(Uav* uav);
    void clearUavs();

    Uav* selectedUav() const;
    const std::vector<Uav*>& uavsList() const;
    size_t uavsCount() const;
    bmcl::OptionPtr<Uav> uav(const mccmsg::Device& name) const;

    bool readyForExchange(const mccmsg::Device& device) const;

    ResponseHandle sendCmdYY(const mccmsg::DevReqPtr& cmd);
    void cancelRequest(mccmsg::RequestId id);
    void cancelRequest(const mccmsg::RequestPtr& req);

    const mccgeo::Geod& geod() const;
    ExchangeService* exchangeService();
    const ExchangeService* exchangeService() const;
    const mccmsg::ProtocolController* protocolController() const;

    void sortUavList();

    double uavPixmapScale() const;

    double calcWaypointAltitudeAt(mccgeo::LatLon latLon, double defaultValue = 0) const;

    bool isUavValid(const Uav* uav);
    bool isUavSelected(const Uav* uav);
    void requestUavDescription(const mccmsg::Device& id);
    void requestUavUnregister(const mccmsg::Device& uav);
    void requestUavUpdate(const mccmsg::Device& name, const bmcl::Option<bool>& regFirst, const bmcl::Option<std::string>& info, const bmcl::Option<bool>& logging, const bmcl::Option<std::string>& settings);
    void requestUavRegister(const QString& info, const mccmsg::ProtocolId& id, QWidget* parent);
    void requestUavConnect(const mccmsg::Device& uav, const mccmsg::Channel& channel, bool state);
    void requestUavActivate(const mccmsg::Device& uav, bool isActive);
    void requestUavAndChannelRegister(const mccmsg::DeviceDescription& d, const mccmsg::ChannelDescription& c, QWidget* parent);

public slots:
    void selectUav(Uav* uav);
    void activateUav(Uav* uav, bool isActive);
    void centerOnUav(const Uav* uav);

    void onUavRegistered(const mccmsg::Device& uav);
    void onUavUnregistered(const mccmsg::Device& uav);
    void onUavUpdated(const mccmsg::DeviceDescription&);

    void onUavStatistics(const mccmsg::StatDevice& statesList);

    void onTraitRouteState(const mccmsg::TmRoutePtr& routeState);
    void onTraitRoutesList(const mccmsg::TmRoutesListPtr& routesList);
    void onUavActivated(const mccmsg::Device& device, bool isActive);
    void onSetTmView(const bmcl::Rc<const mccmsg::ITmView>& view);
    void onUpdateTmStatusView(const bmcl::Rc<const mccmsg::ITmViewUpdate>& update);

    void saveSettings();

    void onRequestAdded(const mccmsg::DevReqPtr& req);
    void onRequestStateChanged(const mccmsg::DevReqPtr& req, const mccmsg::Request_StatePtr&);
    void onRequestRemoved(const mccmsg::DevReqPtr& req);

    void onLog(bmcl::LogLevel logLevel, const mccmsg::Device& device, const std::string& text);
    void onLog(const mccmsg::Device& uav, const std::string& text);
    void onLog(bmcl::LogLevel logLevel, const std::string& text);
    void onLog(const std::string& text);

signals:
    void updateUavSettings(const mccmsg::Device& device, const std::string& settings);

    void requestUavFileUpload(const QString& service, const QString& uav, const QString& filePath);
    void requestUavFileUploadCancel(const QString& service, const QString& uav, const QString& filePath, const QString& reason);
    void log(const mccmsg::tm::LogPtr& logMessage);

    // TODO: remove later
    //////////////////////////////////////////////////////////////////////////
    void setTmView(const bmcl::Rc<const mccmsg::ITmView>& view);
    void updateTmStatusView(const bmcl::Rc<const mccmsg::ITmViewUpdate>& update);
    void tmPaketResponse(const bmcl::Rc<const mccmsg::ITmPacketResponse>& pkt);
    void traitCalibration(const mccmsg::TmCalibrationPtr& calibration);
    void traitCommonCalibrationStatus(const mccmsg::TmCommonCalibrationStatusPtr&);
    //////////////////////////////////////////////////////////////////////////

    void uavAdded(Uav* uav);
    void uavRemoved(Uav* uav);
    void uavSignalBad(Uav* uav);
    void uavSignalGood(Uav* uav);
    void uavFirmwareLoaded(Uav* uav);
    void uavTmStorageUpdated(Uav* uav);
    void uavDescriptionChanged(const Uav* uav);
    void uavRemovingCompleted();
    void uavActivated(Uav* uav, bool isActive);
    void uavStateChanged(Uav* uav);
    void uavListReordered();

    void uavReadyForExchange(Uav* uav);
    void uavNotReadyForExchange(Uav* uav);

    void uavCentering(const mccgeo::LatLon& latLon);
    void selectionChanged(Uav* selectedUav);

    void beginEditTemplate(mccuav::WaypointTempalteType type);
    void endEditTemplate(double delta, double height, double speed);
    void resetEditTemplate(mccuav::WaypointTempalteType type);

    void showTrackSettingsForUav(Uav* uav);
    void showUavAlert(Uav* uav, const QString& message);

    void guiUpdated(Uav* uav, const QString& filename);
    void exportTm(Uav* uav, const QString& outputDir);
    void playTm(Uav* deice, const bmcl::Option<bmcl::SystemTime>& from, const bmcl::Option<bmcl::SystemTime>& to);

    void showPoiEditor(const mccuav::PointOfInterestPtr& point);

    void exportUiArchive(const QString& path, const QString& defaultName);

private slots:
    void updateUavChannels();

private:
    void requestFirmware(const mccmsg::Firmware& f);
    void onUavDescription(const mccmsg::DeviceDescription& uav);
    void onFirmwareDescription(const mccmsg::FirmwareDescription& traits);

    enum class CancelingReason
    {
//        UavAdding = 1,
//        UavRemoving,
        UavSelecting,
        UavActivating,
        UavDeactivating,
        UavUnregistering,
        UavAndChannelUnregistering,
    };
    void addUav(const mccmsg::DeviceDescription& id, bool setCurrent = false);
    void removeUav(Uav* uav);

    void tryCancelRouteEditing(Uav* uav, CancelingReason reason);
    void requestFileList(Uav* uav);
    virtual void timerEvent(QTimerEvent *) override;
    QColor findFreeUavColor() const;

    template <typename F, typename... A>
    void forwardToUav(const mccmsg::Device& device, F&& func, A&&... args);
    void executeUavSelecting(Uav* uav);
    void executeUavActivation(Uav* uav);
    void executeUavDeactivation(Uav* uav);
    void executeUavUnregistering(Uav* uav);
    void executeUavAndChannelUnregistering(Uav* uav);
    void processUavRemoving(Uav* uav);

    std::vector<Uav*>                   _uavs;
    std::set<mccmsg::Device>            _uavsForExchange;

    Uav*                                _selectedUav;

    bool                                _offlineMode;

    std::vector<UiFileDescriptor>       _uiFiles;

    mccgeo::EnuPositionHandler          _enuConverter;
    QMessageBox*                        _messageBox;
    Rc<mccui::Settings>                 _settings;
    Rc<mccui::SettingsWriter>           _activeUavWriter;
    Rc<ChannelsController>              _chanController;
    Rc<RoutesController>                _routesController;
    Rc<const mccmsg::ProtocolController> _protocolController;
    Rc<const mccui::HeightmapController> _hmController;
    Rc<mccuav::ExchangeService>         _exchangeService;

    std::vector<mccmsg::Device>         _deviceDescriptionRequested;
    mccgeo::Geod                        _geod;
    unsigned                            _colorCounter;
    double                              _uavPixmapScale;
    Rc<const mcchm::HmReader> _hmReader;

    Q_DISABLE_COPY(UavController)
};

class MCC_UAV_DECLSPEC UavControllerPluginData : public mccplugin::PluginData {
public:
    static constexpr const char* id = "mcc::UavControllerPluginData";

    UavControllerPluginData(UavController* uavController);
    ~UavControllerPluginData();

    UavController* uavController();
    const UavController* uavController() const;

private:
    Rc<UavController> _uavController;
};
}

