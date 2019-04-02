#pragma once

#include <bmcl/Uuid.h>

#include "mcc/Config.h"
#include <QLabel>

class QPixmap;

namespace mccui {

class MCC_UI_DECLSPEC ClickableLabel : public QLabel
{
    Q_OBJECT

public:
    explicit ClickableLabel(QWidget *parent = nullptr);
    explicit ClickableLabel(const QString &text, QWidget *parent = nullptr);
    ClickableLabel(const QPixmap& pixmap,
                   const QPixmap& hoveredPixmap,
                   QWidget* parent = nullptr);
    ClickableLabel(const bmcl::Uuid& uuid,
                   const QPixmap& pixmap,
                   const QPixmap& hoveredPixmap,
                   QWidget* parent = nullptr);
    explicit ClickableLabel(const bmcl::Uuid& uuid,
                            QWidget* parent = nullptr);
    ClickableLabel(const bmcl::Uuid& uuid,
                   const QString& text,
                   QWidget* parent = nullptr);
    ~ClickableLabel() override;

    void setUuid(const bmcl::Uuid& uuid) { _uuid = uuid; }
    bmcl::Uuid uuid() const { return _uuid; }

    void forceHover(bool hovered);

public slots:
    void setMainPixmap(const QPixmap &pixmap);
    void setHoveredPixmap(const QPixmap &pixmap);
    void removeMainPixmap();
    void removeHoveredPixmap();
    void setHoverableText(const QString& text);

signals:
    void clicked(const QMouseEvent* event);
    void doubleClicked(const QMouseEvent* event);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    QPixmap*        _mainPixmap;
    QPixmap*        _hoveredPixmap;
    bool            _isHovered;

    bmcl::Uuid      _uuid;

    Q_DISABLE_COPY(ClickableLabel)
};
}
