#include "mcc/ui/ColorButton.h"
#include "mcc/ui/ColorDialogOptions.h"

#include <QColorDialog>

namespace mccui {

ColorButton::ColorButton()
    : ColorButton(Qt::white)
{
}

ColorButton::ColorButton(const QColor& color)
{
    setButtonColor(color);
    connect(this, &ColorButton::clicked, this, &ColorButton::selectColor);
}


ColorButton::~ColorButton()
{
}


void ColorButton::setButtonColor(const QColor& color)
{
    _color = color;
    updateColorIcon();
}

const QColor& ColorButton::buttonColor() const
{
    return _color;
}

void ColorButton::selectColor()
{
    QColorDialog::ColorDialogOptions isNative = mccui::colorDialogOptions();
    QColor color = QColorDialog::getColor(_color, this, "Цвет", isNative | QColorDialog::ShowAlphaChannel);
    if (color.isValid()) {
        setButtonColor(color);
        emit colorSelected(_color);
    }
}

void ColorButton::updateColorIcon()
{
    setStyleSheet(QString("background-color: %1;"
                          "border: 1px;"
                          "border-color: black;"
                          "border-style: outset;").arg(_color.name()));
}
}
