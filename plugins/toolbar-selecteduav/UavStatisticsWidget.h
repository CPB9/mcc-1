#pragma once

#include "AbstractUavWidget.h"

namespace mccide{
class NetStatisticsWidget;
}
namespace mccmsg{
class StatDevice;
}

class UavStatisticsWidget : public AbstractUavWidget
{
    Q_OBJECT
public:
    explicit UavStatisticsWidget(QWidget* separator = nullptr, QWidget *parent = nullptr);
    ~UavStatisticsWidget() override;

    bool mayToShow() const override;

    void setStatistics(const mccmsg::StatDevice& stat);

private:
    mccide::NetStatisticsWidget*        _statisticsWidget;

    Q_DISABLE_COPY(UavStatisticsWidget)
};
