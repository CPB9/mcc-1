#pragma once

#include "mcc/Config.h"
#include <QWidget>

#include "mcc/uav/Rc.h"
#include "mcc/uav/Fwd.h"
#include "mcc/msg/Objects.h"
#include "mcc/plugin/Fwd.h"

class QStackedLayout;
class QLabel;

namespace mccide {

class MCC_IDE_DECLSPEC UavBrowserTool : public QWidget
{
    Q_OBJECT

public:
    UavBrowserTool(mccuav::ChannelsController* chanController,
                      mccuav::UavController* uavController,
                      QWidget* parent = nullptr);
    ~UavBrowserTool() override;

    void loadPlugins(const mccplugin::PluginCache* cache);
private slots:
    void selectionChanged(const mccuav::Uav* selectedUav);

private:
    QStackedLayout*                         _mainLayout;
    QLabel*                                 _problemLabel;
    std::map<mccmsg::Protocol, int>         _browserIndices;
    std::map<mccmsg::Protocol, std::string> _protocols;
    mccuav::Rc<mccuav::ChannelsController>  _chanController;
    mccuav::Rc<mccuav::UavController>       _uavController;
};
}
