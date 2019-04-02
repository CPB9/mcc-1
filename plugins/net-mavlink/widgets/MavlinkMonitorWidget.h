#pragma once

#include <QWidget>
#include <set>

#include "mcc/msg/FwdExt.h"
#include "mcc/ui/Rc.h"
#include "mcc/uav/Fwd.h"
#include "mcc/ui/Fwd.h"

class QTreeView;

namespace mccmav {

class MonitorModel;

class MavlinkMonitorWidget : public QWidget
{
    Q_OBJECT

public:
    MavlinkMonitorWidget(mccuav::UavController* uavController, QWidget* parent);
    ~MavlinkMonitorWidget();

private slots:
    void selectionChanged(mccuav::Uav* uav);
    //void tmParamList(const mccmsg::TmParamListPtr& params);

private:
    QTreeView* _widget;
    MonitorModel* _model;
    mccuav::Uav* _uav;
    mccui::Rc<mccuav::UavController> _uavController;
    std::set<std::string> _params;
protected:
    virtual void timerEvent(QTimerEvent *event) override;

};
}
