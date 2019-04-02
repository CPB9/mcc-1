#pragma once

#include "mcc/vis/Config.h"
#include "mcc/vis/Rc.h"

#include <QWidget>

class QTabWidget;
class QTableView;
class QProgressDialog;

namespace mccvis {

class Region;
class ProfileViewer;
class ProfileDataViewer;
class RegionViewer;
class DirectionListModel;
class ReportGen;

class MCC_VIS_DECLSPEC ResultsWidget : public QWidget {
    Q_OBJECT
public:
    ResultsWidget(const Region* region);
    ~ResultsWidget();

    void setRegion(const Region* region);

private slots:
    void selectProfile(std::size_t i);

private:
    void updateView();

    QTableView* _directionsView;
    QTabWidget* _tabWidget;
    RegionViewer* _regionViewer;
    RegionViewer* _angleViewer;
    ProfileViewer* _profileViewer;
    ProfileDataViewer* _dataViewer;
    DirectionListModel* _directionsModel;
    Rc<const Region> _region;
    ReportGen* _gen;
    QProgressDialog* _dialog;
};

}
