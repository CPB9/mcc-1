#pragma once

#include "mcc/Config.h"

#include <QStyledItemDelegate>

class ChannelsListDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit ChannelsListDelegate(QObject *parent = nullptr);
    ~ChannelsListDelegate() override;

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;

    static constexpr QSize defaulSize() {return QSize(68, 48);}
    static constexpr int defaultOffset() {return 4;}

private:
    QFont           _font;

    Q_DISABLE_COPY(ChannelsListDelegate)
};
