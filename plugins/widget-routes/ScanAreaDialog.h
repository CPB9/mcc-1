#pragma once

#include "mcc/ui/Dialog.h"
#include "mcc/uav/WaypointTemplateType.h"

namespace Ui { class ScanAreaDialog; }

class ScanAreaDialog : public mccui::Dialog
{
public:
    explicit ScanAreaDialog(QWidget* parent = 0);
    ~ScanAreaDialog();

    inline mccuav::WaypointTempalteType type() const { return _type; }

    void setType(mccuav::WaypointTempalteType type);
    double delta() const;
    double height() const;
    double speed() const;

private:
    Ui::ScanAreaDialog* _ui;
    mccuav::WaypointTempalteType _type;
};
