#include "StopwatchTimerBaseWidget.h"
#include "StopwatchTimerStatusWidget.h"
#include "StopwatchTimerTool.h"

#include "mcc/ui/Settings.h"
#include "mcc/ide/toolbar/MainToolBar.h"

#include <QVBoxLayout>
#include <QIcon>

StopwatchTimerTool::StopwatchTimerTool(mccui::Settings* settings,
                                       StopwatchTimerStatusWidget* informator,
                                       QWidget* parent)
    : mccui::Dialog(parent)
    , _timers()
    , _informator(informator)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);

    setObjectName("Секундомер");
    setWindowTitle("Секундомер");
    setWindowIcon(QIcon(":/toolbar-timer/resources/timer_icon.png"));

    setStyleSheet(QString(
        "QWidget\n"
        "{\n"
        "	color: #909090;\n" // fafafa
        "	background-color: #%1;\n"
        "}\n\n"
    ).arg(mccide::MainToolBar::mainBackgroundColor().rgb(), 6, 16, QLatin1Char('0')));

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(5, 5, 5, 5);

    for(size_t i = 0; i < timersAmount(); ++i)
    {
        if(i > 0)
        {
            QFrame *line = new QFrame();
            line->setFrameShape(QFrame::HLine);
            line->setFrameShadow(QFrame::Sunken);
            layout->addWidget(line);
        }

        StopwatchTimerTypes type = StopwatchTimerTypes::Direct;
        if(i % 2)
            type = StopwatchTimerTypes::Reverse;

        _timers.push_back(new StopwatchTimerBaseWidget(settings, type, _informator, i, this));
        layout->addWidget(_timers[i]);
    }
}

StopwatchTimerTool::~StopwatchTimerTool()
{}

void StopwatchTimerTool::setInformator(StopwatchTimerStatusWidget* informator)
{
    if(_informator == informator)
        return;

    _informator = informator;

    for(auto timer : _timers)
    {
        timer->setInformator(_informator);
    }
}

void StopwatchTimerTool::requestShowing()
{
    showAndMove();
}

void StopwatchTimerTool::showAndMove()
{
    show();

    if(_informator == nullptr)
        return;

    move(_informator->mapToGlobal(QPoint(_informator->width() - width(),
                                         _informator->height())));
}
