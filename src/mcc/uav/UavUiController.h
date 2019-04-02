#pragma once

#include <QObject>
#include <QTemporaryDir>

#include "mcc/ui/Fwd.h"
#include "mcc/ui/QObjectRefCountable.h"

#include "mcc/uav/Fwd.h"
#include "mcc/uav/Rc.h"
#include "mcc/uav/UavController.h"
#include "mcc/uav/ExchangeService.h"
#include "mcc/uav/UavUi.h"
#include "mcc/plugin/PluginData.h"

namespace mccuav
{

class MCC_UAV_DECLSPEC UavUiController : public mccui::QObjectRefCountable<QObject>
{
    Q_OBJECT
public:
    UavUiController(mccui::Settings* settings, mccuav::UavController* uavController, mccuav::ExchangeService* service);

    bmcl::OptionRc<UavUi> ui(mccmsg::Device device) const;
signals:
    void openUi(mccmsg::Device device, const bmcl::OptionRc<UavUi>& ui);

private slots:
    void onUavAdded(mccuav::Uav* uav);
    void onUavRemoved(mccuav::Uav* uav);
    void onUavDescriptionUpdated(const mccuav::Uav* uav);
private:
    void updateUi(mccmsg::Device device, const mccmsg::DeviceUiDescription& description);
    void saveUi(mccmsg::Device device, const QString& filename, const bmcl::SharedBytes& data);
    void checkIfLocalCopyExists(mccmsg::Device device, const QString& filename) const;

    QTemporaryDir _tempDir;
    mccuav::Rc<UavController> _uavController;
    mccuav::Rc<mccuav::ExchangeService> _exchangeService;
    std::map<mccmsg::Device, Rc<UavUi>> _uis;
};

class MCC_UAV_DECLSPEC UavUiControllerPluginData : public mccplugin::PluginData {
public:
    static constexpr const char* id = "mcc::UavUiControllerPluginData";

    UavUiControllerPluginData(UavUiController* uavUiController);
    ~UavUiControllerPluginData();

    UavUiController* uavUiController();
    const UavUiController* uavUiController() const;

private:
    mccuav::Rc<UavUiController> _uavUiController;
};
}
