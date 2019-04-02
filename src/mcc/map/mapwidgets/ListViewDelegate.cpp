#include "mcc/map/mapwidgets/ListViewDelegate.h"
#include "mcc/map/mapwidgets/Methods.h"
#include "mcc/map/mapwidgets/Types.h"

#include <QPainter>

ListViewDelegate::ListViewDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
    , _cornerRadius(12.0)
    , _numberRectWidth(32)
{}

void ListViewDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    painter->save();
        drawBase(painter, option, index);
    painter->restore();
}

void ListViewDelegate::drawBase(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QColor backColor;
    const bool hovered = option.state & QStyle::State_MouseOver;

    bool editing = index.data(EditModeRole).toBool();
    if(editing)
    {
        backColor = backgroundColorEditing(hovered);
    }
    else
    {
        backColor = backgroundColorCommon(hovered);
    }

    if(option.state & QStyle::State_Selected)
    {
        backColor = backgroundColorSelected(hovered);
    }

    QRect backRect = prepareDrawingRect(painter, option, backColor);

    if(editing)
    {
        if(option.state & QStyle::State_Selected)
            painter->setPen(QColor(210, 210, 210, 255));
        else
            painter->setPen(QColor(15, 15, 15, 255));
    }
    else
    {
        painter->setPen(QColor(210, 210, 210, 255));
    }

    QRect numberRect = backRect;
    numberRect.setWidth(_numberRectWidth);
    painter->drawText(numberRect, Qt::AlignCenter, index.data(NumberRole).toString());
}

QRect ListViewDelegate::prepareDrawingRect(QPainter* painter, const QStyleOptionViewItem& option, const QColor& color) const
{
    painter->setRenderHint(QPainter::Antialiasing, true);

    painter->setPen(QColor(0, 0, 0, 0));

    QRect backRect = option.rect;
    backRect.setWidth(sizeHint().width());
    backRect = backRect - QMargins(0, 0, 0, stepSize());

    if(color.isValid())
    {
        painter->setBrush(color);
        painter->drawRoundedRect(backRect, cornerRadius(), cornerRadius());
    }

    return backRect;
}

QSize ListViewDelegate::sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const
{
    return sizeHint();
}

QSize ListViewDelegate::sizeHint()
{
    return QSize(206, 24 + stepSize());
}
