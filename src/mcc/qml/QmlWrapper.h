#pragma once
#include "mcc/Config.h"
#include "mcc/uav/Rc.h"
#include "mcc/uav/Fwd.h"
#include <unordered_map>
#include <QUuid>
#include <bmcl/StringView.h>
#include "mcc/msg/FwdExt.h"
#include "mcc/msg/ParamList.h"
#include "mcc/msg/NetVariant.h"
#include "mcc/uav/PointOfInterest.h"

class QQuickItem;

namespace mccqml {

class FileTransferWrapper;
class VideoImageProvider;
class MjpegVideoSource;
struct FileTransfer;

class MCC_QML_DECLSPEC QmlWrapper : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString currentDevice READ currentDevice NOTIFY currentDeviceChanged)
    Q_PROPERTY(QString currentDeviceName READ currentDeviceName NOTIFY currentDeviceChanged)
    Q_PROPERTY(QObject* currentDeviceObj READ currentDeviceObj NOTIFY currentDeviceChanged)
    Q_PROPERTY(QStringList devicesList READ devicesList NOTIFY devicesListChanged)
    Q_PROPERTY(QList<QObject*> devices READ devices NOTIFY devicesListChanged)
    Q_PROPERTY(QList<QObject*> devicesByGroups READ devicesByGroups NOTIFY groupsChanged)
public:
    struct ParamHelper
    {
        QQuickItem* item;
        bmcl::SystemTime time;
        mccmsg::NetVariant value;
        ParamHelper() {}

        ParamHelper(QQuickItem* item)
            : item(item)
        {
            time = bmcl::SystemClock::now();
        }
    };

    QmlWrapper(mccuav::GroupsController* groupsController,
               mccuav::UavController* uavController,
               QObject* parent);
    ~QmlWrapper();

    QString currentDevice() const;
    QObject* currentDeviceObj() const;
    QString currentDeviceName() const;
    QStringList devicesList() const;
    QList<QObject*> devices() const;
    QList<QObject*> devicesByGroups() const;

    void reset();

    void setDevice(mccuav::Uav* device);

    Q_INVOKABLE int writeVar(const QString& device, int nodeId, int varId, const QVariant& value);
    Q_INVOKABLE int writeVarsList(const QString& device, int nodeId, int varId, const QVariantList& values);
    Q_INVOKABLE int sendCmd(const QString& device, const QString& trait, const QString& name, const QStringList& params);
    Q_INVOKABLE void cancelCmd(const QString& device, int collationId);
    Q_INVOKABLE void uploadFile(const QString& device, const QString& path, const QString& remote);
    Q_INVOKABLE void cancelUploadFile(const QString& device, const QString& path,const QString& remote);
    Q_INVOKABLE void registerMotion(QQuickItem* item);

    Q_INVOKABLE QObject* registerVideoStream(const QString& boundary, const QString& address, int port, const QString& name, bool dropConnection);
    Q_INVOKABLE void setGhostMotion(const QString& uid, float lat, float lon, float yaw);
    Q_INVOKABLE void sendUdpDatagram(const QString& host, int port, const QString& data);
signals:
    void currentDeviceChanged();
    void devicesListChanged();
    void groupsChanged();

    void mavlinkParamChanged(const QString& device, const QString& name, const QVariant& value, int time);

    void cmdState(const QDateTime& time, int collationId, int state, const QString& reason = QString());
    void fileUploadProgressChanged(const QString& device, const QString& path, int progress);
    void fileUploaded(const QString& device, const QString& filePath);
    void fileUploadError(const QString& device, const QString& filePath, const QString& reason);

    // internal signals
//    void requestDeviceFileUpload(const QString& service, const QString& device, const QString& filePath, const QString& remote, mccide::FileTransfer::Direction direction);
//    void requestDeviceFileUploadCancel(const QString& service, const QString& device, const QString& filePath, const QString& remote, const QString& reason);
    void addVarRequest(const QString& nodeId, const QString& varId, const QString& type, int priority);
    void registerVideoProviderInEngine(const QString& name, VideoImageProvider* provider);
    void unregisterVideoProviderInEngine(const QString& name);

private slots:
    void onDeviceFileUploaded(const QString& device, const QString& filePath);
    void onDeviceFileUploadFailed(const QString& device, const QString& filePath, const QString& reason);
private:
    bmcl::Option<mccuav::Uav*> _device;
    bmcl::SystemTime _startupTime;
    std::vector<VideoImageProvider*>                     _videoProviders;
    std::vector<MjpegVideoSource*>                       _videoSources;
    std::vector<mccuav::PointOfInterestPtr>                _pointsOfInterest;
    QQuickItem* _motion;
    mccuav::Rc<mccuav::GroupsController> _groupsController;
    mccuav::Rc<mccuav::UavController> _uavController;
};
}
