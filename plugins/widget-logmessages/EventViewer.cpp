#include "EventViewer.h"
#include "mcc/ide/models/LogMessagesModel.h"
#include "mcc/ide/toolbar/MainToolBar.h"

#include <QScrollBar>
#include <QHeaderView>
#include <QPalette>
#include <QMouseEvent>
#include <QStyledItemDelegate>
#include <QPainter>

class MultiColorLinePainter
{
public:
    MultiColorLinePainter(QPainter* painter, const QRectF& rect)
        : _painter(painter)
        , _metrics(painter->font())
        , _x(0)
        , _y(rect.bottom())
        , _whitespaceWidth(_metrics.width(' '))
    {
    }

    void appendText(const QString& text, const QColor& color)
    {
        _painter->setPen(QPen(color));
        _painter->drawText(_x, _y, text);
        _x += _metrics.width(text);
    }

    void appendWhitespace()
    {
        _x += _whitespaceWidth;
    }

private:
    QPainter* _painter;
    QFontMetrics _metrics;
    int _x;
    int _y;
    int _whitespaceWidth;
};

class LogRowDelegate : public QStyledItemDelegate
{
public:
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        return QSize(option.rect.width(), 12);
    }

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        const size_t _maxCharsCount = 100;

        QColor deviceColor = index.data(mccide::LogMessagesModel::DeviceColorRole).value<QColor>();
        QColor messageColor = index.data(mccide::LogMessagesModel::MessageColorRole).value<QColor>();
        QString deviceName = index.data(mccide::LogMessagesModel::DeviceNameRole).toString();
        QString messageText = index.data(mccide::LogMessagesModel::MessageRole).toString();
        QString time = index.data(mccide::LogMessagesModel::TimeRole).toString();

        QString text = messageText.left(messageText.indexOf('\n')).left(_maxCharsCount);

        MultiColorLinePainter textPainter(painter, option.rect);
        textPainter.appendText(time, QColor(Qt::white));
        textPainter.appendWhitespace();
        textPainter.appendText(deviceName, deviceColor);
        textPainter.appendWhitespace();
        textPainter.appendText(text, messageColor);
    }

};

EventViewer::EventViewer(mccide::LogMessagesModel* logModel, QWidget* parent)
    : QListView(parent)
{
    using mccide::LogMessagesModel;
    setAutoFillBackground(true);
    setModel(logModel);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setItemDelegate(new LogRowDelegate());
    setMouseTracking(true);
//    setResizeMode(QListView::Adjust);
    setUniformItemSizes(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    setMinimumWidth(150);

    setStyleSheet("border: 0px;");

    connect(logModel, &QAbstractTableModel::rowsInserted, this, &EventViewer::scrollDown);
}

EventViewer::~EventViewer() {}

void EventViewer::setMaxCharsCount(size_t count)
{
    _maxCharsCount = count;
}

QSize EventViewer::sizeHint() const
{
    return QSize(500, 48);
}

void EventViewer::setMaxLinesCount(size_t count)
{
    _maxMessagesCount = count;
}

void EventViewer::scrollDown()
{
    scrollToBottom();
}

void EventViewer::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        emit showDetails();
}
