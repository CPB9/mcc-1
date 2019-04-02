#include "UavStatisticsWidget.h"

#include "mcc/ide/toolbar/MainToolBar.h"
#include "mcc/ide/view/NetStatisticsWidget.h"

#include <QHBoxLayout>

UavStatisticsWidget::UavStatisticsWidget(QWidget* separator, QWidget *parent)
    : AbstractUavWidget(separator, parent)
    , _statisticsWidget(new mccide::NetStatisticsWidget(this))
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 3, 0);

    int h = 34;
    _statisticsWidget->setFixedSize(h * 2, h);
    _statisticsWidget->setFocusPolicy(Qt::ClickFocus);
    _statisticsWidget->setStyleSheet("border: 0px; "
                                     "background-color: rgba(0, 0, 0, 255);"
                                     "color: white;");
    _statisticsWidget->setTextColor(Qt::white);
    setMinimumSize(_statisticsWidget->size());
    layout->addWidget(_statisticsWidget);
}

UavStatisticsWidget::~UavStatisticsWidget()
{}

bool UavStatisticsWidget::mayToShow() const
{
    return true;
}

void UavStatisticsWidget::setStatistics(const mccmsg::StatDevice& stat)
{
    _statisticsWidget->updateStats(stat._sent, stat._rcvd, stat._bad);
}
