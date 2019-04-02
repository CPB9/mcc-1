#pragma once

#include <QStyledItemDelegate>

#include "mcc/Config.h"

class MCC_MAP_DECLSPEC ListViewDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit ListViewDelegate(QObject *parent = nullptr);

    static constexpr int stepSize() {return 2;}
    int numberRectWidth() const {return _numberRectWidth;}
    void setNumberRectWidth(int width) {_numberRectWidth = width;}

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;
    static QSize sizeHint();

    double cornerRadius() const {return _cornerRadius;}
    void setCornerRadius(double radius) {_cornerRadius = radius;}

protected:
    void drawBase(QPainter *painter,
                  const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QRect prepareDrawingRect(QPainter *painter, const QStyleOptionViewItem &option, const QColor& color = QColor()) const;

private:
    double  _cornerRadius;
    int     _numberRectWidth;

    Q_DISABLE_COPY(ListViewDelegate)
};
