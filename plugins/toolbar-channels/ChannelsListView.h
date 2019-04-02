#pragma once

#include "mcc/Config.h"

#include <QListView>

class ChannelsListView : public QListView
{
public:
    explicit ChannelsListView(QWidget *parent = nullptr);
    ~ChannelsListView() override;

protected:
    bool event(QEvent *e) override;
    bool viewportEvent(QEvent *event) override;

private:
    Q_DISABLE_COPY(ChannelsListView)
};
