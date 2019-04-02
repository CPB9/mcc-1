#include "ChannelsListDelegate.h"

#include "ChannelsListModel.h"
#include "mcc/ide/toolbar/MainToolBar.h"
#include "mcc/ide/view/NetStatisticsWidget.h"
#include "mcc/ui/TextUtils.h"

#include <QApplication>
#include <QFont>
#include <QPainter>

ChannelsListDelegate::ChannelsListDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
    , _font(QApplication::font())
{
    _font.setPixelSize(11);
}

ChannelsListDelegate::~ChannelsListDelegate()
{}

void ChannelsListDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if(!index.isValid())
        return;

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setFont(_font);

//    painter->setBrush(brush);
//    painter->setPen(QColor(0, 0, 0, 0));

//    painter->drawRoundRect(backRect, 4, 4);
//    painter->setPen(QColor(255, 255, 255, 255));

    mccide::NetStatisticsWidget* statWidget = index.data(ChannelsListModel::StatsWidgetRole).value<mccide::NetStatisticsWidget*>();
    if(statWidget != nullptr)
    {
        QPoint offset = painter->deviceTransform().map(option.rect.topLeft());
        offset.setX(offset.x() + defaultOffset());
        offset.setY(offset.y() + defaultOffset());
        statWidget->render(painter,
                           offset,
                           QRegion());
    }

    int vOffset = option.rect.height() / 3;
    QRect backRect = option.rect - QMargins(defaultOffset(), vOffset, defaultOffset(), vOffset);
    backRect.moveTop(backRect.y() + vOffset / 2);

    QColor brushColor(0, 0, 0, 128);
    QColor penColor(192, 192, 192, 255);
    if(option.state & QStyle::State_MouseOver)
    {
        brushColor = QColor(0, 0, 0, 255);
        penColor = QColor(255, 255, 255, 255);
    }

    painter->setPen(QColor(0, 0, 0, 0));
    painter->setBrush(brushColor);
    painter->drawRoundedRect(backRect - QMargins(1, 0, 1, 0), 4, 4);
    painter->setPen(penColor);
    painter->drawText(backRect, Qt::AlignCenter, mccui::shortTextLine(index.data(Qt::DisplayRole).toString(), 8));

    painter->restore();
}

QSize ChannelsListDelegate::sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const
{
    return defaulSize();
}
