#include "mcc/uav/UavController.h"
#include "mcc/ui/Settings.h"
#include "mcc/uav/RoutesController.h"
#include "mcc/uav/ChannelsController.h"
#include "mcc/uav/GroupsController.h"
#include "mcc/uav/ExchangeService.h"
#include "mcc/msg/ProtocolController.h"
#include "mcc/msg/ptr/Device.h"
#include "mcc/msg/ptr/Advanced.h"
#include "mcc/msg/ptr/ReqVisitor.h"
#include "mcc/msg/Cmd.h"

#include "mcc/hm/HmReader.h"
#include "mcc/ui/HeightmapController.h"
#include "mcc/geo/Constants.h"
#include "mcc/res/Resource.h"

#include "mcc/plugin/PluginCache.h"

#include <bmcl/Logging.h>
#include <bmcl/MakeRc.h>

#include <QApplication>
#include <QStyle>
#include <QPixmap>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QMessageBox>
#include <QAbstractButton>

namespace mccuav {

using mccuav::ExchangeService;

static const char* _warningText = "Данное действие требует завершение редактирования маршрута.\nВыйти из режима редактирования?\nВсе несохраненные изменения будут потеряны.";

UavController::UavController(mccui::Settings* settings,
                             ChannelsController* chanController,
                             RoutesController* routesController,
                             const mccui::HeightmapController* hmController,
                             const mccmsg::ProtocolController* protocolController,
                             mccuav::ExchangeService* service)
    : _selectedUav(nullptr)
    , _offlineMode(false)
    , _messageBox(new QMessageBox())
    , _settings(settings)
    , _chanController(chanController)
    , _routesController(routesController)
    , _hmController(hmController)
    , _protocolController(protocolController)
    , _exchangeService(service)
    , _geod(mccgeo::wgs84a<double>(), mccgeo::wgs84f<double>())
    , _colorCounter(0)
    , _uavPixmapScale(1)
{
    _hmReader = _hmController->cloneHeightmapReader();
    connect(hmController, &mccui::HeightmapController::heightmapReaderChanged, this, [this](const mcchm::Rc<const mcchm::HmReader>& reader) {
        _hmReader = reader;
    });

    _activeUavWriter = settings->acquireUniqueWriter("exchange/activeDevices").unwrap();

    startTimer(1000);

    _messageBox->setIcon(QMessageBox::Warning);
    _messageBox->setWindowTitle("Отмена редактирования маршрута");
    _messageBox->setText(_warningText);
    _messageBox->setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    _messageBox->setDefaultButton(QMessageBox::Ok);
    // With hope that first button is Ok. Because Ok button code (0x00000400) is smaller than Cancel button (0x00400000)
    connect(_messageBox->buttons().first(), &QAbstractButton::clicked, _messageBox, &QMessageBox::accepted);
    connect(_messageBox->buttons().at(1), &QAbstractButton::clicked, _messageBox, &QMessageBox::rejected);
    _messageBox->setModal(true);
    _messageBox->hide();

    auto exchangePtr = _exchangeService.get();

    QString scaleKey = "map/devicePixmapScale";
    _uavPixmapScale = _settings->read(scaleKey, 1.0).toDouble();
    settings->onChange(scaleKey, this, [this](const QVariant& scale) {
        double s = scale.toDouble();
        for (Uav* uav : _uavs) {
            uav->setColor(uav->color(), s);
        }
        _uavPixmapScale = s;
    });

    connect(exchangePtr, &ExchangeService::log, this, &UavController::log);
    connect(exchangePtr, &ExchangeService::traitRouteState, this, &UavController::onTraitRouteState);
    connect(exchangePtr, &ExchangeService::traitRoutesList, this, &UavController::onTraitRoutesList);
    connect(exchangePtr, &ExchangeService::setTmView, this, &UavController::onSetTmView);
    connect(exchangePtr, &ExchangeService::updateTmStatusView, this, &UavController::onUpdateTmStatusView);

    connect(exchangePtr, &ExchangeService::setTmView, this, &UavController::setTmView);
    connect(exchangePtr, &ExchangeService::updateTmStatusView, this, &UavController::updateTmStatusView);
    connect(exchangePtr, &ExchangeService::tmPaketResponse, this, &UavController::tmPaketResponse);
    connect(exchangePtr, &ExchangeService::traitCalibration, this, &UavController::traitCalibration);
    connect(exchangePtr, &ExchangeService::traitCommonCalibrationStatus, this, &UavController::traitCommonCalibrationStatus);

    connect(exchangePtr, &ExchangeService::deviceActivated, this, &UavController::onUavActivated);

    //connect(managerPtr, &UavController::subscribeTmParam, exchangePtr, &ExchangeService::onTmParamSubscribtion);
    //connect(managerPtr, &UavController::showPoiEditor, this, &MainWindow::showPoiEditor);

    connect(exchangePtr, &ExchangeService::deviceUpdated, this, &UavController::onUavUpdated);
    connect(exchangePtr, &ExchangeService::deviceState, this, &UavController::onUavStatistics);
    connect(exchangePtr, &ExchangeService::deviceRegistered, this, &UavController::onUavRegistered);
    connect(exchangePtr, &ExchangeService::deviceUnRegistered, this, &UavController::onUavUnregistered);

    connect(exchangePtr, &ExchangeService::requestAdded, this, &UavController::onRequestAdded);
    connect(exchangePtr, &ExchangeService::requestStateChanged, this, &UavController::onRequestStateChanged);
    connect(exchangePtr, &ExchangeService::requestRemoved, this, &UavController::onRequestRemoved);
//     connect(managerPtr, &UavController::exportTm, this,
//             [this, _exchangeService](mccui::FlyingDevice* dev, const QString& outputDir)
//     {
//         _exchangeService->onTmDump(bmcl::Uuid(dev->name()), outputDir.toStdString(), bmcl::None, bmcl::None, false);
//     }
//     );
//
//     connect(managerPtr, &UavController::playTm, this,
//             [this, _exchangeService](mccui::FlyingDevice* device, const bmcl::Option<bmcl::SystemTime>& from, const bmcl::Option<bmcl::SystemTime>& to)
//     {
//         _exchangeService->onTmPlay(bmcl::Uuid(device->name()), from, to);
//     }
//     );
//
//     connect(managerPtr, &UavController::exportUiArchive, this, &MainWindow::exportUiArchive);
//
//     connect(managerPtr, &UavController::updateDeviceSettings, this,
//             [this](const mccmsg::Device& device, const std::string& settings)
//     {
//         mccui::Context::instance()->exchangeService()->sender()->request<mccmsg::device::Update_Request>(device, bmcl::None, bmcl::None, settings, bmcl::None);
//     }
//     );

    connect(this, &UavController::uavSignalGood, this, [this](Uav* uav) { _exchangeService->onLog(bmcl::LogLevel::Info, uav->device(), "Восстановлена связь");});
    connect(this, &UavController::uavSignalBad, this,  [this](Uav* uav) { _exchangeService->onLog(bmcl::LogLevel::Warning, uav->device(), "Потеряна связь");});

    connect(this, &UavController::uavAdded, this, &UavController::updateUavChannels);
    connect(chanController, &ChannelsController::channelsChanged, this, &UavController::updateUavChannels);

    connect(this, &UavController::selectionChanged, routesController, &RoutesController::acceptUavSelection);
    connect(this, &UavController::uavRemoved, routesController, &RoutesController::acceptUavRemoving);

    connect(this, &UavController::showUavAlert, this, [](Uav* uav, const QString& message) { BMCL_DEBUG() << "Потерялася я! Ахтунг!"; });
}

void UavController::updateUavChannels()
{
    std::map<mccmsg::Device, mccmsg::Channels> devicesChannels;
    for (const ChannelInformation& channel : _chanController->channelInformations())
    {
        if(channel.channelDescription().isNone())
            continue;

        for (const auto& d : channel.channelDescription().unwrap()->connectedDevices())
        {
            auto it = devicesChannels.find(d.device());
            if (it == devicesChannels.end())
            {
                devicesChannels.insert(std::make_pair(d.device(), mccmsg::Channels()));
                it = devicesChannels.find(d.device());
            }
            it->second.push_back(channel.channel());
        }
    }

    for (const auto& channels : devicesChannels)
    {
        bmcl::OptionPtr<Uav> dev = this->uav(channels.first);
        if (dev.isSome())
            dev->setChannels(channels.second);
    }
}

Uav* UavController::selectedUav() const
{
    return _selectedUav;
}

void UavController::selectUav(Uav* device)
{
    if (_selectedUav == device)
        return;

    if (_routesController->isEditing())
        tryCancelRouteEditing(device, CancelingReason::UavSelecting);
    else
        executeUavSelecting(device);
}

void UavController::addUav(const mccmsg::DeviceDescription& id, bool setCurrent /*= false*/)
{
    if(this->uav(id->name()).isSome())
    {
        BMCL_ASSERT(false);
        return;
    }

    Uav* device = new Uav(this, id);
    device->setSourcePixmapFile(mccres::loadResource(mccres::ResourceKind::DeviceUnknownIcon));
    device->setColor(findFreeUavColor(), _uavPixmapScale);

    _uavs.push_back(device);

    if (setCurrent)
        selectUav(device);

    auto activeDevices = _activeUavWriter->read().toStringList();
    if (activeDevices.indexOf(id->name().toQString()) != -1)
    {
        requestUavActivate(id->name(), true);
    }

    emit uavAdded(device);

    selectUav(device);
}

void UavController::removeUav(Uav* device)
{
    Q_ASSERT(device != nullptr);
    BMCL_DEBUG() << "Removing device " << device->device().toStdString();

    auto it = std::find_if(_uavs.begin(), _uavs.end(), [device](const Uav* d) {return d->device() == device->device(); });
    if (it == _uavs.end())
    {
        BMCL_ASSERT(false);
        return;
    }

    _uavs.erase(it);
    processUavRemoving(device);
}

void UavController::unregisterUavAndChannel(Uav* uav)
{
    if(uav == nullptr)
        return;

    if(_routesController->isEditing())
        tryCancelRouteEditing(uav, CancelingReason::UavAndChannelUnregistering);
    else
        executeUavAndChannelUnregistering(uav);
}

void UavController::unregisterUav(Uav* uav)
{
    if(uav == nullptr)
        return;

    if(_routesController->isEditing())
        tryCancelRouteEditing(uav, CancelingReason::UavUnregistering);
    else
        executeUavUnregistering(uav);
}

UavController::~UavController()
{
    disconnect();

    for (auto device : _uavs)
    {
        delete device;
    }

    delete _messageBox;
}

void UavController::clearUavs()
{
    selectUav(nullptr);

    for (auto device : _uavs)
    {
        qDebug() << "UavController::deviceRemoved";
        processUavRemoving(device);
    }

    _uavs.clear();
    _uavsForExchange.clear();
}

const std::vector<Uav*>& UavController::uavsList() const
{
    return _uavs;
}

size_t UavController::uavsCount() const
{
    return _uavs.size();
}

void UavController::requestFileList(Uav* device)
{
    sendCmdYY(new mccmsg::CmdFileGetList(device->device())).then([](const mccmsg::CmdRespAnyPtr& rep)
    {
        auto pp = bmcl::dynamic_pointer_cast<const mccmsg::CmdFileGetListResp>(rep);
        assert(!pp.isNull());
        if (pp.isNull())
            return;
        pp->files();
        pp->ui();
    });
}

ResponseHandle UavController::sendCmdYY(const mccmsg::DevReqPtr& cmd)
{
    return ResponseHandle(_exchangeService.get(), cmd);
}

void UavController::cancelRequest(mccmsg::RequestId id)
{
    if (_exchangeService.isNull())
        return;
    _exchangeService->cancel(id);
}

void UavController::cancelRequest(const mccmsg::RequestPtr& req)
{
    if (_exchangeService.isNull())
        return;
    _exchangeService->cancel(req);
}

ResponseHandle::ResponseHandle(mccuav::ExchangeService* service, mccmsg::DevReqPtr&& r)
    : _sent(false), _service(service), _r(r)
{
}

ResponseHandle::ResponseHandle(mccuav::ExchangeService* service, const mccmsg::DevReqPtr& r)
    : _sent(false), _service(service), _r(r)
{
}

mccmsg::DevReqPtr ResponseHandle::then(OnSuccessCmd&& f, OnError&& e, OnState&& s)
{
    _sent = true;
    if (!f)
        f = [](const mccmsg::ResponsePtr &) {};
    return _service->requestXX(_r.get()).then(std::move(f), std::move(e), std::move(s));
}

ResponseHandle::~ResponseHandle()
{
    if (!_sent)
    {
        BMCL_DEBUG() << "ответ не обрабатывается";
        auto r = _service->requestXX(_r.get()).then([](const mccmsg::CmdRespAnyPtr &) {});
    }
}

void UavController::sortUavList()
{
    std::stable_sort(_uavs.begin(), _uavs.end(),
                     [](Uav* left, Uav* right)
                     {
                        if (left->protocol() == right->protocol())
                            return left->deviceId() < right->deviceId();

                        return (left->protocol() < right->protocol());
                     }
    );

    emit uavListReordered();
}

bool UavController::isUavValid(const Uav* uav)
{
    return _uavs.end() != std::find_if(_uavs.begin(), _uavs.end(), [uav](const Uav* i){ return i == uav; });
}

bool UavController::isUavSelected(const Uav* uav)
{
    return uav == _selectedUav;
}

void UavController::requestUavUnregister(const mccmsg::Device& id)
{
    _exchangeService->requestXX(new mccmsg::device::UnRegister_Request(id)).then
    (
        [this](const mccmsg::device::UnRegister_ResponsePtr& rep)
        {
        }
    );
}

void UavController::requestUavDescription(const mccmsg::Device& id)
{
    if (std::find(_deviceDescriptionRequested.begin(), _deviceDescriptionRequested.end(), id) != _deviceDescriptionRequested.end())
        return;

    _exchangeService->requestXX(new mccmsg::device::Description_Request(id)).then
    (
        [this](const mccmsg::device::Description_ResponsePtr& rep)
        {
            onUavDescription(rep->data());

            auto it = std::find(_deviceDescriptionRequested.begin(), _deviceDescriptionRequested.end(), rep->data()->name());
            if (it != _deviceDescriptionRequested.end())
                _deviceDescriptionRequested.erase(it);
        }
    );

    _deviceDescriptionRequested.push_back(id);
}

void UavController::requestUavUpdate(const mccmsg::Device& name, const bmcl::Option<bool>& regFirst, const bmcl::Option<std::string>& info, const bmcl::Option<bool>& logging)
{
    auto d = bmcl::makeRc<const mccmsg::DeviceDescriptionObj>(name, info.unwrapOr(""), "", mccmsg::ProtocolId(), bmcl::None,
                                                              bmcl::SharedBytes(), bmcl::None, regFirst.unwrapOr(false), logging.unwrapOr(false));
    mccmsg::Fields fs;
    if (regFirst.isSome()) fs.push_back(mccmsg::Field::RegFirst);
    if (info.isSome()) fs.push_back(mccmsg::Field::Info);
    if (logging.isSome()) fs.push_back(mccmsg::Field::Log);

    _exchangeService->requestXX(new mccmsg::device::Update_Request(mccmsg::device::Updater(std::move(d), std::move(fs)))).then
    (
        [](const mccmsg::device::Update_ResponsePtr& rep)
        {

        },
        [](const mccmsg::ErrorDscr& err)
        {

        }
    );
}

void UavController::requestUavRegister(const QString& info, const mccmsg::ProtocolId& id, QWidget* parent)
{
    auto dscr = bmcl::makeRc<const mccmsg::DeviceDescriptionObj>(mccmsg::Device(), info.toStdString(), std::string(), id, bmcl::None, bmcl::SharedBytes(), bmcl::None, false, false);
    _exchangeService->requestXX(new mccmsg::device::Register_Request(dscr)).then
    (
        [](const mccmsg::device::Register_ResponsePtr& rep)
        {

        },
        [this, info, parent](const mccmsg::ErrorDscr& err)
        {
            QMessageBox::warning(parent, QString("Ошибка при регистрации аппарата %1").arg(info),
                QString("Ошибка при регистрации аппарата %1: %2").arg(info).arg(err.qfull()));

        }
    );
}

void UavController::requestUavConnect(const mccmsg::Device& device, const mccmsg::Channel& channel, bool state)
{
    _exchangeService->requestXX(new mccmsg::device::Connect_Request(device, channel, state)).then
    (
        [](const mccmsg::device::Connect_ResponsePtr& rep)
        {

        },
        [](const mccmsg::ErrorDscr& err)
        {

        }
    );
}

void UavController::requestUavActivate(const mccmsg::Device& device, bool isActive)
{
    auto d = device;
    bool i = isActive;
    _exchangeService->requestXX(new mccmsg::device::Activate_Request(device, isActive)).then
    (
        [this, d, i](const mccmsg::device::Activate_ResponsePtr& rep)
        {
            onUavActivated(d, i);
        },
        [](const mccmsg::ErrorDscr& err)
        {

        }
    );

    QStringList values = _activeUavWriter->read().toStringList();
    int index = values.indexOf(device.toQString());
    if (index != -1)
    {
        values.removeAt(index);
    }
    if (isActive)
        values << device.toQString();
    _activeUavWriter->write(values);
}

void UavController::requestUavAndChannelRegister(const mccmsg::DeviceDescription& d, const mccmsg::ChannelDescription& c, QWidget* parent)
{
    _exchangeService->requestXX(new mccmsg::advanced::ChannelAndDeviceRegister_Request(d, c)).then
    (
        [this](const mccmsg::advanced::ChannelAndDeviceRegister_ResponsePtr&)
        {
            BMCL_DEBUG() << "Channel created";
        }
      , [this, parent](const mccmsg::ErrorDscr& e)
        {
            QMessageBox::warning(parent, "Ошибка при создании аппарата", QString("Ошибка при создании аппарата: %1").arg(e.qfull()));
        }
    );
}

void UavController::init()
{
}

void UavController::timerEvent(QTimerEvent *)
{
    if (_offlineMode)
        return;

    const int INACTIVE_STATE = 3;

    QDateTime currentDateTime = QDateTime::currentDateTime();

    for (auto device : _uavs)
    {
        if (device->lastTmMsgDateTime().isNull())
        {
            QDateTime pastDateTime = currentDateTime.addSecs(-INACTIVE_STATE - 1);

            device->setLastMsgTime(pastDateTime);
            device->setAlive(false);
            device->setSignalBad();
            emit(uavSignalBad(device));
            continue;
        }

        int inactiveSecs = device->lastTmMsgDateTime().secsTo(currentDateTime);
        if ((inactiveSecs > INACTIVE_STATE) && device->isAlive())
        {
            device->setAlive(false);
            device->setSignalBad();
            emit(uavSignalBad(device));
        }

        if ((inactiveSecs <= INACTIVE_STATE) && !device->isAlive())
        {
            device->setAlive(true);
            device->setSignalGood();
            emit(uavSignalGood(device));
        }

        auto it = _uavsForExchange.find(device->device());
        if (!device->isStateActive() && it != _uavsForExchange.end())
        {
            emit uavNotReadyForExchange(device);
            _uavsForExchange.erase(it);
        }
    }
}

QColor UavController::findFreeUavColor() const
{
    const char *colors[] =
    {
        "Yellow",
        "LightSalmon",
        "SteelBlue",
        "Fuchsia",
        "HotPink",
        "Peru",
        "Maroon"
    };

    const int numColors = sizeof(colors) / sizeof(colors[0]);
    auto isColorInUse = [this](const QColor& c)
    {
        for (auto it : _uavs)
        {
            if (it->color() == c)
                return true;
        }
        return false;
    };

    for (int i = 0; i < numColors; ++i)
    {
        if (!isColorInUse(colors[i]))
            return colors[i];
    }
    return colors[_uavs.size() % numColors];
}

void UavController::executeUavSelecting(Uav* device)
{
    _selectedUav = device;
    emit selectionChanged(device);

    // Single route autoselecting
    if(selectedUav() != nullptr && selectedUav()->selectedRoute() == nullptr)
    {
        const QVector<Route*>& routes = selectedUav()->routes();
        if(routes.size() == 1)
            selectedUav()->selectRoute(routes[0]);
    }
}

void UavController::executeUavActivation(Uav* device)
{
    if(device == nullptr)
        return;
    requestUavActivate(device->device(), true);
}

void UavController::executeUavDeactivation(Uav* device)
{
    if(device == nullptr)
        return;
    requestUavActivate(device->device(), false);
}

void UavController::executeUavUnregistering(Uav* device)
{
    if(device == nullptr)
        return;

    mccmsg::Device d = device->device();
    _exchangeService->requestXX(new mccmsg::device::UnRegister_Request(device->device())).then
    (
        [this, d](const mccmsg::device::UnRegister_ResponsePtr& rep)
        {
            onUavUnregistered(d);
        }
    );
}

void UavController::executeUavAndChannelUnregistering(Uav* device)
{
    if(device == nullptr)
        return;

    mccmsg::Device d = device->device();
    mccmsg::Channel channelId = device->channels().front();
    _exchangeService->requestXX(new mccmsg::device::UnRegister_Request(device->device())).then(
        [channelId, this](const mccmsg::device::UnRegister_ResponsePtr&)
        {
            _chanController->requestChannelUnregister(channelId, nullptr);
        }
      , [d](const mccmsg::ErrorDscr&)
        {
            BMCL_WARNING() << "Устройство " << d.toStdString() << " отсутствует в бд";
        }
    );

}

void UavController::processUavRemoving(Uav* device)
{
    if(device == nullptr)
        return;

    if(device == selectedUav())
        selectUav(nullptr);

    emit uavRemoved(device);
    _uavsForExchange.erase(device->device());
    delete device;
    emit uavRemovingCompleted();
}

void UavController::onUavUpdated(const mccmsg::DeviceDescription& device)
{
    onUavDescription(device);
}

void UavController::onUavRegistered(const mccmsg::Device& device)
{
    requestUavDescription(device);
}

void UavController::onUavUnregistered(const mccmsg::Device& device)
{
    auto uav = this->uav(device);
    if (uav.isNone())
    {
        return;
    }
    removeUav(uav.unwrap());
}

void UavController::onUavStatistics(const mccmsg::StatDevice& deviceState)
{
    if (!_chanController->isUavInChannel(deviceState._device))
    {
        const auto i = std::find_if(_uavs.begin(), _uavs.end(), [&deviceState](const Uav* uav) { return deviceState._device == uav->device(); });
        if (i != _uavs.end())
        {
            auto uav = *i;
            _uavs.erase(i);
            processUavRemoving(uav);
        }
        return;
    }

    auto uav = this->uav(deviceState._device);
    if (uav.isNone())
    {
        requestUavDescription(deviceState._device);
        return;
    }

    uav->setStatDevice(deviceState);

    if (deviceState._isActive /*&& deviceState.isRegistered()*/ && (_uavsForExchange.find(uav->device()) == _uavsForExchange.end()))
    {
        emit uavReadyForExchange(uav.unwrap());

        _uavsForExchange.insert(uav->device());
        requestFileList(uav.unwrap());
    }
}

void UavController::onUavDescription(const mccmsg::DeviceDescription& description)
{
    auto d = uav(description->name());
    if (d.isNone())
    {
        addUav(description);
        onUavRegistered(description->name());
        d = uav(description->name());
    }

    d->setDeviceDescription(description);
    emit uavDescriptionChanged(d.unwrap());

    sortUavList();

    if (description->firmware().isSome())
        requestFirmware(description->firmware().unwrap());

    if (description->pixmap().isNull())
    {
        assert(false);
    }
    else
    {
        d->setSourcePixmapFile(description->pixmap().view());
    }

    sendCmdYY(new mccmsg::CmdGetTmView(d->device()));
}

void UavController::requestFirmware(const mccmsg::Firmware& f)
{
    _exchangeService->requestXX(new mccmsg::firmware::Description_Request(f)).then
    (
        [this](const mccmsg::firmware::Description_ResponsePtr& rep)
        {
            onFirmwareDescription(rep->data());
        }
      , [f](const mccmsg::ErrorDscr&)
        {
            BMCL_WARNING() << "Прошивка " << f.toStdString() << " отсутствует в бд";
        }
    );
}

void UavController::onFirmwareDescription(const mccmsg::FirmwareDescription& frm)
{
    for (auto dev : _uavs)
    {
        if (dev->deviceDescription()->firmware().isNone() || (dev->deviceDescription()->firmware().unwrap() != frm->name()))
            continue;

        if(dev->firmwareDescription().isNone() || (dev->firmwareDescription()->name() != frm->name()))
        {
            dev->setFirmwareDescription(frm);
            emit uavFirmwareLoaded(dev);
            sendCmdYY(new mccmsg::CmdRouteGetList(dev->device()));
        }
    }
}

void UavController::saveSettings()
{
    //for (auto dev : _devices.values())
    //    dev->saveSettings();
}


void UavController::onRequestAdded(const mccmsg::DevReqPtr& req)
{
    for (auto dev : _uavs)
    {
        dev->onCmdAdded(req);
    }
}

void UavController::onRequestStateChanged(const mccmsg::DevReqPtr& req, const mccmsg::Request_StatePtr& state)
{
    for (auto dev : _uavs)
    {
        dev->onCmdStateChanged(req, state);
    }
}

void UavController::onRequestRemoved(const mccmsg::DevReqPtr& req)
{
    for (auto dev : _uavs)
    {
        dev->onCmdRemoved(req);
    }
}

void UavController::onLog(bmcl::LogLevel logLevel, const mccmsg::Device& device, const std::string& text)
{
    _exchangeService->onLog(logLevel, device, text);
}

void UavController::onLog(const mccmsg::Device& device, const std::string& text)
{
    _exchangeService->onLog(device, text);
}

void UavController::onLog(const std::string& text)
{
    _exchangeService->onLog(text);
}

void UavController::onLog(bmcl::LogLevel logLevel, const std::string& text)
{
    _exchangeService->onLog(logLevel, text);
}

void UavController::activateUav(Uav* device, bool isActive)
{
    if(device == nullptr)
        return;

    CancelingReason reason(isActive ? CancelingReason::UavActivating : CancelingReason::UavDeactivating);

    if (_routesController->isEditing())
        tryCancelRouteEditing(device, reason);
    else
    {
        if(isActive)
            executeUavActivation(device);
        else
            executeUavDeactivation(device);
    }
}

void UavController::centerOnUav(const Uav* device)
{
    if(!device || device->position().isNone())
        return;
    emit uavCentering(device->position()->latLon());
}

void UavController::tryCancelRouteEditing(Uav* device, CancelingReason reason)
{
    _messageBox->disconnect();

    switch (reason) {
    case CancelingReason::UavSelecting:
        _messageBox->setText(QString("Смена выбранного аппарата.\n") + _warningText);
        break;
    case CancelingReason::UavActivating:
        _messageBox->setText(QString("Активация аппарата.\n") + _warningText);
        break;
    case CancelingReason::UavDeactivating:
        _messageBox->setText(QString("Деактивация аппарата.\n") + _warningText);
        break;
    case CancelingReason::UavUnregistering:
        _messageBox->setText(QString("Удаление аппарата.\n") + _warningText);
        break;
    case CancelingReason::UavAndChannelUnregistering:
        _messageBox->setText(QString("Удаление аппарата и канала.\n") + _warningText);
        break;
    }

    connect(_messageBox, &QMessageBox::accepted, this,
            [this, device, reason]()
    {
        _routesController->stopEditRoute();

        switch (reason) {
        case CancelingReason::UavSelecting:
            executeUavSelecting(device);
            break;
        case CancelingReason::UavActivating:
            executeUavActivation(device);
            break;
        case CancelingReason::UavDeactivating:
            executeUavDeactivation(device);
            break;
        case CancelingReason::UavUnregistering:
            executeUavUnregistering(device);
            break;
        case CancelingReason::UavAndChannelUnregistering:
            executeUavAndChannelUnregistering(device);
            break;
        }
    });

    _messageBox->show();
}

void UavController::onTraitRouteState(const mccmsg::TmRoutePtr& route)
{
    forwardToUav(route->device(), &Uav::processRouteState, route);
}

void UavController::onTraitRoutesList(const mccmsg::TmRoutesListPtr& routesList)
{
    forwardToUav(routesList->device(), &Uav::processRoutesList, routesList);
}

void UavController::onUavActivated(const mccmsg::Device& device, bool isActive)
{
    forwardToUav(device, &Uav::processActivated, isActive);

    sortUavList();

    auto d = this->uav(device);
    if (d.isSome())
        emit uavActivated(d.unwrap(), isActive);
}

QColor UavController::nextRouteColor()
{
    static const char *colors[] = {
        "LightSalmon",
        "SteelBlue",
        //"Yellow",
        "Fuchsia",
        //"PaleGreen",
        //"PaleTurquoise",
        //"Cornsilk",
        "HotPink",
        "Peru",
        "Maroon"
    };

    constexpr unsigned numColors = sizeof(colors) / sizeof(colors[0]);

    _colorCounter++;
    _colorCounter %= numColors;

    return QColor(colors[_colorCounter]);
}

void UavController::onSetTmView(const bmcl::Rc<const mccmsg::ITmView>& view)
{
    forwardToUav(view->device(), &Uav::processSetTmView, view);
}

void UavController::onUpdateTmStatusView(const bmcl::Rc<const mccmsg::ITmViewUpdate>& update)
{
    forwardToUav(update->device(), &Uav::processUpdateTmStatusView, update);
}

// void UavController::onCmdResponse(const mccmsg::cmd::ItemPtr& req, const bmcl::Result<mccmsg::cmd::ResponsePtr, mccmsg::ErrorDscr>& rep)
// {
//     (void)req;
//     (void)rep;
//                 auto it = std::find_if(_uiFiles.begin(), _uiFiles.end(), [&](const UiFileDescriptor& desc) {return desc.cmdId == state->requestId(); });
//                 if (it == _uiFiles.end())
//                     return;
//
//                 auto device = this->device(it->device);
//                 if (device.isNone())
//                     return;
//
//                 it->cmdState = state->();
//                 switch (state->state())
//                 {
//                 case mccmsg::cmd::State::InProgress:
//                 {
//                     emit(*device)->guiStatusChanged(FlyingDevice::Loading, state->progress());
//                     break;
//                 }
//                 case mccmsg::cmd::State::Failed:
//                 {
//                     qDebug() << "Ошибка при загрузке файла интерфейса: " << QString::fromStdString(it->file.file);
//                     emit(*device)->guiStatusChanged(FlyingDevice::Failed, 0);
//                     (*device)->setUiFile("", FlyingDevice::Failed);
//                     _uiFiles.erase(it);
//                     break;
//                 }
//                 case mccmsg::cmd::State::Done:
//                 {
//                     qDebug() << "Файл с интерфейсом загружен";
//                     QString filename = QString::number(it->file.crc.unwrap(), 16);
//                     (*device)->setUiFile(filename, FlyingDevice::Done);
//                     emit guiUpdated(device.unwrap(), QString::fromStdString(it->path));
//                     emit(*device)->guiStatusChanged(FlyingDevice::Done, 100);
//                     _uiFiles.erase(it);
//                     break;
//                 }
//                 default:
//                     break;
//                 }
//
//}

bool UavController::readyForExchange(const mccmsg::Device& device) const
{
    (void)device;
    //return _devicesForExchange.contains(device) && _devices[device]->isStateActive();
    return true;
}

bmcl::OptionPtr<Uav> UavController::uav(const mccmsg::Device& name) const
{
    const auto it = std::find_if(_uavs.begin(), _uavs.end(), [name](const Uav* uav) { return uav->device() == name; });
    if(it == _uavs.end())
        return bmcl::None;
    return *it;
}

template <typename F, typename... A>
void UavController::forwardToUav(const mccmsg::Device& device, F&& func, A&&... args)
{
    auto d = uav(device);
    if (d.isNone())
        return;

    (d.unwrap()->*func)(std::forward<A>(args)...);
}

UavControllerPluginData::UavControllerPluginData(UavController* uavController)
    : mccplugin::PluginData(id)
    , _uavController(uavController)
{
}

UavControllerPluginData::~UavControllerPluginData()
{
}

UavController* UavControllerPluginData::uavController()
{
    return _uavController.get();
}

const UavController* UavControllerPluginData::uavController() const
{
    return _uavController.get();
}

ExchangeService* UavController::exchangeService()
{
    return _exchangeService.get();
}

const ExchangeService* UavController::exchangeService() const
{
    return _exchangeService.get();
}

const mccmsg::ProtocolController* UavController::protocolController() const
{
    return _protocolController.get();
}

const mccgeo::Geod& UavController::geod() const
{
    return _geod;
}

double UavController::uavPixmapScale() const
{
    return _uavPixmapScale;
}

double UavController::calcWaypointAltitudeAt(mccgeo::LatLon latLon, double defaultValue) const
{
    return _hmReader->readAltitude(latLon).unwrapOr(defaultValue);
}
}
