#pragma once

#include "mcc/Config.h"

#include "mcc/msg/obj/Tm.h"

#include "mcc/uav/Rc.h"
#include "mcc/ide/ui_MccPlotTool.h"
#include "mcc/uav/Fwd.h"

#include <QWidget>

namespace mccide {

class TraitsModel;
class PlotTraitsModel;

class MccPlotWidget : public QWidget
{
public:
    MccPlotWidget(mccuav::UavController* uavController, QWidget* parent = 0);
    ~MccPlotWidget();

    void saveSettings();
    private slots:
    void tmParamList(const mccmsg::TmParamList& params);
    void tmParamSubscribeChanged(TmParamTreeItem* item, bool subscribe);

    void timeIntervalChanged(int index);

    void loadSettings(mccuav::Uav* dev);
    void deviceActivated(mccuav::Uav* dev);
private:
    mccuav::Rc<mccuav::UavController> _uavController;
    QColor findFreeColor() const;

    Ui::MccPlotTool _ui;

    mccuav::UavController* _uavController;

    TraitsModel* _model;
    PlotTraitsModel* _proxyModel;

    QVector<QString> _listenParams;

    QVector<TmParamTreeItem*> _startReadQueue;
    QVector<TmParamTreeItem*> _stopReadQueue;
    bool _isLoadingSettings;
};
}
