#pragma once

#include "mcc/Config.h"

#include <QToolButton>
#include <QColor>

namespace mccui {

class MCC_UI_DECLSPEC ColorButton : public QToolButton {
    Q_OBJECT
public:
    ColorButton();
    ColorButton(const QColor& color);
    ~ColorButton();

    void setButtonColor(const QColor& color);
    const QColor& buttonColor() const;

signals:
    void colorSelected(const QColor& color);

private slots:
    void selectColor();

private:
    void updateColorIcon();

    QColor _color;
};
}
