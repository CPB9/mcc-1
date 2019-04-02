#pragma once

#include <QWidget>

#include "mcc/ui/Rc.h"
#include "mcc/ui/Fwd.h"
#include "mcc/uav/Fwd.h"
#include "mcc/msg/FwdExt.h"

class QListView;
class QTableView;
class QLineEdit;
class QStringListModel;
class QSortFilterProxyModel;

namespace mccmav {

class FirmwareModel;
class ParameterEditor;
class DrawWithoutFocusDelegate;

class MavlinkParametersWidget : public QWidget
{
    Q_OBJECT
public:
    MavlinkParametersWidget(mccuav::ChannelsController* chanController,
                            mccuav::UavController* uavController,
                            QWidget* parent);
    ~MavlinkParametersWidget();

    void selectionChanged(mccuav::Uav* uav);
    void firmwareLoaded(mccuav::Uav* uav);
//private slots:
//    void tmParamList(const mccmsg::TmParamListPtr& params);

private:
    mccuav::Uav* _uav;
    FirmwareModel* _model;
    QSortFilterProxyModel* _sortModel;
    QStringListModel* _categoriesModel;

    QTableView* _view;
    QListView* _categories;
    QLineEdit* _filter;

    ParameterEditor*    _parameterEditor;
    DrawWithoutFocusDelegate* _drawWithoutFocusDelegate;
    mccui::Rc<mccuav::ChannelsController> _chanController;
    mccui::Rc<mccuav::UavController> _uavController;
};
}
