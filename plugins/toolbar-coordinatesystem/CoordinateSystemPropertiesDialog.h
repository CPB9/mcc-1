#pragma once

#include "mcc/Config.h"
#include "mcc/uav/Fwd.h"
#include "mcc/ui/Dialog.h"
#include "mcc/ui/Fwd.h"
#include "mcc/ui/Rc.h"

class QListWidget;
class QPushButton;

class CoordinateSystemPropertiesDialog : public mccui::Dialog
{
    Q_OBJECT

public:
    explicit CoordinateSystemPropertiesDialog(mccui::CoordinateSystemController* csController,
                                              mccuav::GlobalActions* actions,
                                              QWidget *parent = nullptr);
    ~CoordinateSystemPropertiesDialog() override;

public slots:
    void updateSystemAndFormat();

private:
    mccui::Rc<mccui::CoordinateSystemController>    _csController;
    mccui::Rc<mccuav::GlobalActions>                _actions;

    QListWidget*                                    _systemList;
    QListWidget*                                    _formatList;
    QPushButton*                                    _converterButton;

    Q_DISABLE_COPY(CoordinateSystemPropertiesDialog)
};
