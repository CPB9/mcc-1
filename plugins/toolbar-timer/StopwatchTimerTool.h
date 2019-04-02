#pragma once

#include "mcc/ui/Rc.h"
#include "mcc/ui/Fwd.h"
#include "mcc/ui/Dialog.h"

#include <vector>

class StopwatchTimerBaseWidget;
class StopwatchTimerStatusWidget;

class StopwatchTimerTool : public mccui::Dialog
{
    Q_OBJECT

public:
    explicit StopwatchTimerTool(mccui::Settings* settings,
                                StopwatchTimerStatusWidget* informator = nullptr,
                                QWidget* parent = nullptr);
    ~StopwatchTimerTool() override;

    void setInformator(StopwatchTimerStatusWidget* informator);
    StopwatchTimerStatusWidget* informator() const {return _informator;}
    size_t timersAmount() const {return 2;}

public slots:
    void requestShowing();

private:
    void showAndMove();

    std::vector<StopwatchTimerBaseWidget*>  _timers;
    StopwatchTimerStatusWidget*             _informator;

private:
    Q_DISABLE_COPY(StopwatchTimerTool)
};
