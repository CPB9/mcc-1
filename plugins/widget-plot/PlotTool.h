#pragma once
#include <QWidget>
#include <bmcl/Option.h>
#include "mcc/msg/FwdExt.h"
#include "mcc/uav/Rc.h"
#include "mcc/uav/Fwd.h"

class PlotWidget;

class PlotTool : public QWidget
{
    Q_OBJECT
public:
    PlotTool(mccuav::UavController* uavController, QWidget* parent = 0);
    ~PlotTool();

//private slots:
//    void tmParamList(const mccmsg::TmParamListPtr& params);
private:
    QColor findFreeColor() const;

private:
    mccuav::Rc<mccuav::UavController> _uavController;
    PlotWidget* _plot;
protected:
    virtual void dragEnterEvent(QDragEnterEvent *event) override;
    virtual void dropEvent(QDropEvent *event) override;
};
