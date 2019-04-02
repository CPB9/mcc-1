#include "ChannelsListView.h"

#include <QEvent>
#include <QScrollBar>

ChannelsListView::ChannelsListView(QWidget* parent)
    : QListView (parent)
{
    setFlow(QListView::LeftToRight);

    setStyleSheet("border: none;");

    horizontalScrollBar()->setDisabled(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    verticalScrollBar()->setDisabled(true);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

ChannelsListView::~ChannelsListView()
{}

bool ChannelsListView::event(QEvent* e)
{
    if(e->type() == QEvent::Scroll ||
       e->type() == QEvent::ScrollPrepare ||
       e->type() == QEvent::KeyPress ||
       e->type() == QEvent::MouseButtonPress ||
       e->type() == QEvent::MouseMove)
    {
        e->ignore();
        return true;
    }

    return QListView::event(e);
}

bool ChannelsListView::viewportEvent(QEvent* event)
{
    event->ignore();
    return QListView::viewportEvent(event);
}
