#pragma once

#include <QWidget>

#include "mcc/uav/Fwd.h"
#include "mcc/ui/Fwd.h"

class QLabel;

class DeviceItem : public QWidget
{
public:
    DeviceItem(mccuav::UavController* controller, mccuav::Uav* uav, QWidget* parent = nullptr);
    mccuav::Uav* uav() const;
    void updateData();
private:
    mccuav::Uav* _uav;
    QLabel* _name;
    mccui::OnOffSliderCheckBox* _slider;
};
