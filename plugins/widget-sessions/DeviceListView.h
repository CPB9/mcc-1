#pragma once

#include <QWidget>

#include "mcc/ui/Rc.h"
#include "mcc/uav/UavController.h"

class QScrollArea;

class DeviceItem;

class DeviceListView : public QWidget
{
    Q_OBJECT
public:
    DeviceListView(const mccui::Rc<mccuav::UavController>& uavController);
    bool empty() const;
private slots:
    void onUavAdded(mccuav::Uav* uav);
    void onUavRemoved(mccuav::Uav* uav);
    void onUavUpdated(mccuav::Uav* uav);

protected:
    virtual void showEvent(QShowEvent *event) override;

private:
    mccui::Rc<mccuav::UavController> _controller;
    QScrollArea* _view;
    std::vector<DeviceItem*> _widgets;

};

