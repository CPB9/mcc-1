#pragma once

#include "mcc/Config.h"

#include <QFrame>

class QLabel;

namespace mccui{
class ClickableLabel;
}

namespace mccide {

class MCC_IDE_DECLSPEC AddEntityWidget : public QFrame
{
    Q_OBJECT
public:
    explicit AddEntityWidget(const QImage& main, const QImage& hovered, QWidget *parent = nullptr);
    explicit AddEntityWidget(QWidget *parent = nullptr);
    ~AddEntityWidget() override;

    bool eventFilter(QObject *watched, QEvent *event) override;
    void setIcon(const QImage& main, const QImage& hovered = QImage());

signals:
    void clicked(const QMouseEvent* event);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    mccui::ClickableLabel*  _iconLabel;
    QLabel*                 _textLabel;

    Q_DISABLE_COPY(AddEntityWidget)
};
}
