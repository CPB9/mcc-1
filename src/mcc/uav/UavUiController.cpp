#include "UavUiController.h"
#include "mcc/uav/Uav.h"
#include "mcc/uav/UavUi.h"


#include <QDir>
#include <QStandardPaths>

namespace mccuav {

UavUiController::UavUiController(mccui::Settings* settings, mccuav::UavController* uavController, mccuav::ExchangeService* service)
    : _uavController(uavController)
    , _exchangeService(service)
{
    connect(uavController, &UavController::uavAdded, this, &UavUiController::onUavAdded);
    connect(uavController, &UavController::uavRemoved, this, &UavUiController::onUavRemoved);
    connect(uavController, &UavController::uavDescriptionChanged, this, &UavUiController::onUavDescriptionUpdated);
}

bmcl::OptionRc<mccuav::UavUi> UavUiController::ui(mccmsg::Device device) const
{
    auto it = _uis.find(device);
    if(it == _uis.end())
        return bmcl::None;
    return it->second;
}

void UavUiController::onUavAdded(mccuav::Uav* uav)
{

}

void UavUiController::onUavRemoved(mccuav::Uav* uav)
{

}

void UavUiController::onUavDescriptionUpdated(const mccuav::Uav* uav)
{
    const auto& description = uav->deviceDescription();
    if (description->ui().isNone())
    {
        //TODO: подчистить неиспользуемый интерфейс (если он есть)
        emit openUi(uav->device(), bmcl::None);
        return;
    }

    _exchangeService->requestXX(new mccmsg::deviceUi::Description_Request(description->ui().unwrap())).then
    (
        [this, uav](const mccmsg::deviceUi::Description_ResponsePtr& rep)
        {
            updateUi(uav->device(), rep->data());
        }
    );
}

void UavUiController::updateUi(mccmsg::Device device, const mccmsg::DeviceUiDescription& description)
{
    if(description->data().isEmpty())
    {
//        assert(false);
        BMCL_WARNING() << "Получен файл с нулевым размером";
        return;
    }
    saveUi(device, description->name().toQString(), description->data());
}

void UavUiController::saveUi(mccmsg::Device device, const QString& filename, const bmcl::SharedBytes& data)
{
    auto existsFile = this->ui(device);
    if(existsFile.isSome() && existsFile->name() == filename)
        return;

    bmcl::Rc<UavUi> ui = new UavUi(_tempDir, filename);
    auto res = ui->extractAndValidate(filename, data);
    if (res.isErr())
    {
        auto err = res.takeErr();
        switch(err)
        {
        case UiExtractError::BadZipArchive:
            _uavController->onLog(bmcl::LogLevel::Critical, device, "Ошибка при открытии архива с интерфейсом");
            break;
        case UiExtractError::BrokenZipArchive:
            _uavController->onLog(bmcl::LogLevel::Critical, device, "Ошибка при распаковке архива с интерфейсом");
            break;
        case UiExtractError::MainFileNotFound:
            _uavController->onLog(bmcl::LogLevel::Critical, device, "В архиве с интерфейсом отсутствует файл main.qml");
            break;
        }
        emit openUi(device, bmcl::None);
        return;
    }

    if(ui->localCopyExists())
        ui->setType(UavUi::Type::LocalCopy);
    else
        ui->setType(UavUi::Type::Onboard);

    _uis[device] = ui;
    emit openUi(device, ui);
}

void UavUiController::checkIfLocalCopyExists(mccmsg::Device device, const QString& filename) const
{

}

UavUiControllerPluginData::UavUiControllerPluginData(UavUiController* uavUiController)
    : mccplugin::PluginData(id)
    , _uavUiController(uavUiController)
{

}

UavUiControllerPluginData::~UavUiControllerPluginData()
{

}

mccuav::UavUiController* UavUiControllerPluginData::uavUiController()
{
    return _uavUiController.get();
}

const mccuav::UavUiController* UavUiControllerPluginData::uavUiController() const
{
    return _uavUiController.get();
}

}

