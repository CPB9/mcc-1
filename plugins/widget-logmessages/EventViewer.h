#pragma once
#include <QListView>
#include "mcc/ide/models/LogMessagesModel.h"

class EventViewer : public QListView
{
    Q_OBJECT
public:
    EventViewer(mccide::LogMessagesModel* logModel, QWidget* parent = nullptr);
    ~EventViewer() override;

    void setMaxLinesCount(size_t count);
    void setMaxCharsCount(size_t count);

    virtual QSize sizeHint() const override;

signals:
    void showDetails();

private slots:
    void scrollDown();

protected:
    virtual void mouseReleaseEvent(QMouseEvent *event) override;

private:
    size_t                                  _maxMessagesCount = 3;
    size_t                                  _maxCharsCount = 100;
    std::deque<mccide::AbstractLogMessage>  _messages;
};
