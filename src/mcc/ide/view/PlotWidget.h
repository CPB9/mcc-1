#pragma once
#include <QWidget>
#include <bmcl/Option.h>
#include "mcc/msg/FwdExt.h"
#include "mcc/uav/Rc.h"
#include "mcc/uav/Fwd.h"

namespace mccide {

class MccPlot;

class MCC_IDE_DECLSPEC PlotWidget : public QWidget
{
    Q_OBJECT
public:
    PlotWidget(mccuav::UavController* uavController, QWidget* parent = 0);
    ~PlotWidget();

//private slots:
//    void tmParamList(const mccmsg::TmParamListPtr& params);
private:
    QColor findFreeColor() const;

private:
    mccuav::Rc<mccuav::UavController> _uavController;
    MccPlot* _plot;
protected:
    virtual void dragEnterEvent(QDragEnterEvent *event) override;
    virtual void dropEvent(QDropEvent *event) override;
};
}
