#include "mcc/qml/QmlWrapper.h"

#include <unordered_map>

#include <QObject>
#include <QQuickItem>
#include <QApplication>
#include <QStyle>
#include <QQmlEngine>

#include "mcc/Config.h"
#include "mcc/map/FlagRenderer.h"
#include "mcc/uav/GroupsController.h"
#include "mcc/uav/UavController.h"
#include "mcc/qml/QmlDeviceGroup.h"

//#include "mcc/qml/FileTransferWrapper.h"
#include "mcc/qml/VideoImageProvider.h"

#include "mcc/msg/ptr/Tm.h"

#include "mcc/uav/PointOfInterest.h"
//#include "mcc/ui/map/drawables/Flag.h"

#include "mcc/qml/MjpegVideoSourceTcp.h"
#include "mcc/qml/MjpegVideoSourceUdp.h"

namespace mccqml {

QmlWrapper::QmlWrapper(mccuav::GroupsController* groupsController,
                       mccuav::UavController* uavController,
                       QObject* parent)
    : QObject(parent)
    , _motion(nullptr)
    , _groupsController(groupsController)
    , _uavController(uavController)
{
    _startupTime = bmcl::SystemClock::now();
    //connect(wrapper, &FileTransferWrapper::deviceFileLoaded,            this, &QmlWrapper::onDeviceFileUploaded);
    //connect(wrapper, &FileTransferWrapper::deviceFileLoadFailed,        this, &QmlWrapper::onDeviceFileUploadFailed);
    //connect(wrapper, &FileTransferWrapper::fileUploadProgressChanged,   this, &QmlWrapper::fileUploadProgressChanged);

    //connect(this, &QmlWrapper::requestDeviceFileUpload, wrapper, &FileTransferWrapper::onDeviceFileLoad);
    //connect(this, &QmlWrapper::requestDeviceFileUploadCancel, wrapper, &FileTransferWrapper::onDeviceFileLoadCancel);

    connect(_uavController.get(), &mccuav::UavController::selectionChanged, this, &QmlWrapper::currentDeviceChanged);

    //connect(exchange, &mccuav::ExchangeService::traitNavigationMotion, this, &QmlWrapper::onTraitNavigationMotion);
    connect(_uavController.get(), &mccuav::UavController::uavAdded, this, &QmlWrapper::devicesListChanged);
    connect(_uavController.get(), &mccuav::UavController::uavRemoved, this, &QmlWrapper::devicesListChanged);

    connect(_uavController.get(), &mccuav::UavController::uavAdded, this, &QmlWrapper::groupsChanged);
    connect(_uavController.get(), &mccuav::UavController::uavRemoved, this, &QmlWrapper::groupsChanged);
    connect(_groupsController.get(), &mccuav::GroupsController::groupAdded, this, &QmlWrapper::groupsChanged);
    connect(_groupsController.get(), &mccuav::GroupsController::groupChanged, this, &QmlWrapper::groupsChanged);
    connect(_groupsController.get(), &mccuav::GroupsController::groupRemoved, this, &QmlWrapper::groupsChanged);

    emit groupsChanged();
}

QmlWrapper::~QmlWrapper()
{
    for (auto src : _videoSources)
    {
        delete src;
    }
    _videoSources.clear();
}

QString QmlWrapper::currentDevice() const
{
    auto dev = _uavController->selectedUav();
    if (dev)
        return dev->device().toQString();
    return QString();
}

QObject* QmlWrapper::currentDeviceObj() const
{
    return _uavController->selectedUav();
}

QString QmlWrapper::currentDeviceName() const
{
    auto dev = _uavController->selectedUav();
    if (dev)
    {
        return dev->getInfo();
    }
    return "<Не выбрано>";
}

QStringList QmlWrapper::devicesList() const
{
    QStringList devicesUids;

    const auto& devs = _uavController->uavsList();
    for (const auto& dev : devs)
    {
        devicesUids << dev->device().toQString();
    }
    return devicesUids;
}

QList<QObject*> QmlWrapper::devices() const
{
    QList<QObject*> result;

    auto sortedDevices = _uavController->uavsList();
    std::sort(sortedDevices.begin(), sortedDevices.end(), [](const mccuav::Uav* a, const mccuav::Uav* b) -> bool
        {
            return a->deviceId() < b->deviceId();
        }
    );

    for (auto& dev : sortedDevices)
    {
        result.append((QObject*)dev);
    }

    return result;
}

QList<QObject*> QmlWrapper::devicesByGroups() const
{
    QList<QmlDeviceGroup*> groups;
    std::map<int32_t, size_t> groupsKeys;

    for (const auto& d : _uavController->uavsList())
    {
        mccmsg::GroupId groupId = d->groupId().unwrapOr(-1);
        QmlDeviceGroup* tmp;
        if (groupsKeys.find(groupId) == groupsKeys.end())
        {
            groupsKeys[groupId] = groups.length();
            tmp = new QmlDeviceGroup(groupId);
            groups.append(tmp);
        }
        tmp = groups[groupsKeys[groupId]];
        tmp->addDevice(d);
    }

    QList<QObject*> tmp;
    for (auto t : groups)
    {
        tmp.append(t);
    }
    return tmp;
}

int QmlWrapper::writeVar(const QString& device, int nodeId, int varId, const QVariant& value)
{
    //_uavController->sendCmd<mccmsg::CmdParamList>(mccmsg::Device(cmdItem->device()), cmdItem->trait().toStdString(), cmdItem->name().toStdString(), params);

//    auto r = _uavController->sendCmd(mccmsg::CmdParamList(mccmsg::Device(device), "Sbus", "writeVariable", { nodeId, varId, 1, value.toString() }));
//    return (int)r->message_id();
    return -1;
}

int QmlWrapper::writeVarsList(const QString& device, int nodeId, int varId, const QVariantList& values)
{
    mccmsg::cmd::Params params = { nodeId, varId, values.count() };
    for (auto value : values)
    {
        params.push_back(value.toString());
    }

    return -1;
    //auto r = _uavController->sendCmd(mccmsg::CmdParamList(mccmsg::Device(device), "Sbus", "writeVariable", params));
//    return (int)r->message_id();
}

int QmlWrapper::sendCmd(const QString& device, const QString& trait, const QString& name, const QStringList& params)
{
    mccmsg::CmdParams ps;
    for (const auto& i : params)
    {
        ps.push_back(i);
    }
    auto guid = device;

    const auto& devices = _uavController->uavsList();
    auto deviceByNameIt = std::find_if(devices.begin(), devices.end(), [device](mccuav::Uav* dev) { return dev->getInfo() == device; });
    if (deviceByNameIt != devices.end())
        guid = (*deviceByNameIt)->device().toQString();

    auto r = _uavController->sendCmdYY(new mccmsg::CmdParamList(mccmsg::Device(guid), trait.toStdString(), name.toStdString(), ps)).then();
    return (int)r->requestId();
}

void QmlWrapper::cancelCmd(const QString& device, int collationId)
{
    //_uavController->cancelCmd(mcc::messages::Device(device), collationId);
}

void QmlWrapper::uploadFile(const QString& device, const QString& path, const QString& remote)
{
//    emit requestDeviceFileUpload("", device, path, remote, mccide::FileTransfer::Direction::Up);
}

void QmlWrapper::cancelUploadFile(const QString& device, const QString& path,const QString& remote)
{
//    emit requestDeviceFileUploadCancel("", device, path, remote, "User cancel");
}

Q_INVOKABLE void QmlWrapper::registerMotion(QQuickItem* item)
{
    _motion = item;
}

void QmlWrapper::reset()
{
    _pointsOfInterest.clear();
    if(_device.isSome())
        (*_device)->clearPointsOfInterest();

    for (auto provider : _videoProviders)
    {
        emit unregisterVideoProviderInEngine(provider->name());
    }

    _videoProviders.clear();

    for (auto src : _videoSources)
    {
        delete src;
    }
    _videoSources.clear();
}

void QmlWrapper::setDevice(mccuav::Uav* device)
{
    _device = device;
    (*_device)->clearPointsOfInterest();
    for (const auto& it : _pointsOfInterest)
    {
        (*_device)->addPointOfInterest(it);
    }

    connect(device, &mccuav::Uav::motionChanged, this, &QmlWrapper::onTraitNavigationMotion);
}

QObject* QmlWrapper::registerVideoStream(const QString& boundary, const QString& address, int port, const QString& name, bool dropConnection)
{
    QString providerAddr = name.mid(QString("image://").length());
    MjpegVideoSource* src = nullptr;
    if (address.isEmpty())
        src = new MjpegVideoSourceUdp(boundary, port);
    else
        src = new MjpegVideoSourceTcp(address, port, dropConnection);

    VideoImageProvider* videoProvider = new VideoImageProvider(providerAddr, src);
    QQmlEngine::setObjectOwnership(videoProvider, QQmlEngine::CppOwnership);

    _videoProviders.push_back(videoProvider);
    _videoSources.push_back(src);

    emit registerVideoProviderInEngine(providerAddr, videoProvider);
    return videoProvider;
}

Q_INVOKABLE void QmlWrapper::setGhostMotion(const QString& device, float lat, float lon, float heading)
{
    //auto guid = device;
    //
    //const auto& devices = _uavController->devicesList();
    //auto deviceByNameIt = std::find_if(devices.begin(), devices.end(), [device](mccui::FlyingDevice* dev) { return dev->getInfo() == device; });
    //if (deviceByNameIt != devices.end())
    //    guid = (*deviceByNameIt)->name();
    //
    //auto devPtr = _uavController->device(guid);
    //if (devPtr.isNone())
    //{
    //    qDebug() << "QmlWrapper::setGhostMotion: can't find device " << guid;
    //    return;
    //}
    //(*devPtr)->setGhostMotionParams(lat, lon, heading);
}

Q_INVOKABLE void QmlWrapper::sendUdpDatagram(const QString& host, int port, const QString& text)
{
    QUdpSocket s;
    s.writeDatagram(text.toUtf8(), QHostAddress(host), port);
}

void QmlWrapper::onDeviceFileUploaded(const QString& device, const QString& filePath)
{
    emit fileUploadProgressChanged(device, filePath, 100);
    emit fileUploaded(device, filePath);
}

void QmlWrapper::onDeviceFileUploadFailed(const QString& device, const QString& filePath, const QString& reason)
{
    emit fileUploadError(device, filePath, reason);
}

void QmlWrapper::onTraitNavigationMotion(const mccmsg::Motion& m)
{
    if (!_motion)
        return;

    _motion->setProperty("latitude", m.position.latitude());
    _motion->setProperty("longitude", m.position.latitude());
    _motion->setProperty("heading", m.orientation.heading());
    _motion->setProperty("pitch", m.orientation.pitch());
    _motion->setProperty("roll", m.orientation.roll());
}

// void QmlWrapper::onTmParamList(const mccmsg::TmParamListPtr& paramsList)
// {
//     if (_uavController->selectedUav() == nullptr ||
//         paramsList->device() != _uavController->selectedUav()->device())
//         return;
// 
//     for (const auto& p : paramsList->params())
//     {
//         if (_params.find(p) == _params.end())
//             continue;
//         _params[p].time = paramsList->time();
//         _params[p].value = p.value();
//     }
// }

// void QmlWrapper::onTmParamList2(const mccmsg::TmParamListPtr& paramsList)
// {
//     for (const auto& p : paramsList->params())
//     {
//         if (p.trait() == "Mavlink.Params")
//         {
//             //qDebug() << QDateTime::currentDateTime() << "Mavlink.Params";
//             emit mavlinkParamChanged(paramsList->device().toQString(), p.status().c_str(), p.value().toQVariant(), 0);
//         }
// 
//         auto it = std::find_if(_params2.begin(), _params2.end(), [&paramsList, &p](const ParamHelperWithDevice& one) { return one.device == paramsList->device().toQUuid() && one.tm == p; });
//         if (it == _params2.end())
//             continue;
// 
//         it->time = paramsList->time();
//         it->tm.set_value(p.value());
//     }
// }

}
