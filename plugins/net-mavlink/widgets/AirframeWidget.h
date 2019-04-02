#pragma once
#include <QWidget>
#include <bmcl/Option.h>
#include "mcc/msg/FwdExt.h"
#include "mcc/uav/Rc.h"
#include "mcc/uav/Fwd.h"
#include "../device/Tm.h"

class QLabel;
class QListView;

namespace mccmav {

class AirframesModel;

class AirframeWidget : public QWidget
{
    Q_OBJECT
public:
    AirframeWidget(const mccmsg::Protocol& protocol, mccuav::UavController* uavController, QWidget* parent);
    ~AirframeWidget();

private slots:
    void selectionChanged(mccuav::Uav* uav);

    void setAirframeId(int af);

    //void tmParamList(const mccmsg::TmParamListPtr& params);
private:
    void loadXmlModel();

    mccmsg::Protocol _protocol;
    mccuav::Rc<mccuav::UavController> _uavController;
    bmcl::Option<int> _airframeIndex;
    mccuav::Uav* _uav;
    AirframesModel* _model;
    QLabel* _currentAirframeLabel;
    QLabel* _rebootWarning;
    QListView* _airframesView;
    bmcl::Rc<TmStorage> _tmStorage;
};
}

