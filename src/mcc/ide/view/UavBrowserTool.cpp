#include "mcc/ide/view/UavBrowserTool.h"

#include "mcc/uav/UavController.h"
#include "mcc/uav/Uav.h"
#include "mcc/uav/ChannelsController.h"

#include "mcc/plugin/PluginCache.h"
#include "mcc/uav/FirmwareWidgetPlugin.h"

#include <bmcl/Logging.h>

#include <QLabel>
#include <QDebug>
#include <QInputDialog>
#include <QFormLayout>
#include <QStackedLayout>
#include <QPushButton>

namespace mccide {

UavBrowserTool::UavBrowserTool(mccuav::ChannelsController* chanController,
                                     mccuav::UavController* uavController,
                                     QWidget* parent)
    : QWidget(parent)
    , _chanController(chanController)
    , _uavController(uavController)
{
    setObjectName("Конфигуратор");
    setWindowTitle(objectName());
    setWindowIcon(QIcon(":/uavbrowser-icon.png"));

    _mainLayout = new QStackedLayout();
    _problemLabel = new QLabel("Не выбран аппарат / не загружено описание прошивки для аппарата");
    _mainLayout->addWidget(_problemLabel);


    connect(uavController, &mccuav::UavController::selectionChanged, this, &UavBrowserTool::selectionChanged);
    connect(uavController, &mccuav::UavController::uavFirmwareLoaded, this,
            [this](mccuav::Uav* uav) {
                if(uav == _uavController->selectedUav())
                    selectionChanged(uav);
            }
    );
    connect(uavController, &mccuav::UavController::uavDescriptionChanged, this,
            [this](const mccuav::Uav* uav) {
                if (uav == _uavController->selectedUav())
                    selectionChanged(uav);
            }
    );

    connect(_chanController.get(), &mccuav::ChannelsController::protocolsChanged, this,
            [this]()
            {
                selectionChanged(_uavController->selectedUav());
            }
    );
    setLayout(_mainLayout);
}

UavBrowserTool::~UavBrowserTool()
{
}

void UavBrowserTool::loadPlugins(const mccplugin::PluginCache* cache)
{
    int index = 1;
    for (const auto& plugin : cache->plugins())
    {
        if (!plugin->hasTypeId(mccuav::FirmwareWidgetPlugin::id)) {
            continue;
        }
        auto p = static_cast<mccuav::FirmwareWidgetPlugin*>(plugin.get());
        auto w = p->takeWidget();
        _mainLayout->addWidget(w.release());
        _browserIndices[p->protocol()] = index;
        index++;
    }
}

void UavBrowserTool::selectionChanged(const mccuav::Uav* selectedUav)
{
    if (selectedUav == nullptr)
    {
        _problemLabel->setText("Не выбран аппарат");
        _mainLayout->setCurrentIndex(0);
        return;
    }

    if(selectedUav->firmwareDescription().isNone())
    {
        _problemLabel->setText("Отсутствует прошивка");
        _mainLayout->setCurrentIndex(0);
        return;
    }

    if (selectedUav->deviceDescription()->firmware().isNone())
    {
        _problemLabel->setText("Не загружено описание прошивки");
        _mainLayout->setCurrentIndex(0);
        return;
    }

    auto protocol = selectedUav->firmwareDescription()->id().protocol();
    auto protocolDescription = _chanController->protocolDescription(protocol);
    if (protocolDescription.isNone())
    {
        _problemLabel->setText("Отсутствует описание протокола обмена");
        _mainLayout->setCurrentIndex(0);
        return;
    }

    auto browserIndex = _browserIndices.find(protocolDescription.unwrap()->name());
    if (browserIndex == _browserIndices.end())
    {
        _problemLabel->setText("Отсутствует виджет для данного аппарата");
        _mainLayout->setCurrentIndex(0);
        BMCL_DEBUG() << "UavBrowser: Unknown widget";
        return;
    }

    _mainLayout->setCurrentIndex(browserIndex->second);
}
}
