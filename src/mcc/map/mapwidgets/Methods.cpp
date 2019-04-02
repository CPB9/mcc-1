#include "mcc/map/mapwidgets/Methods.h"
#include "mcc/map/mapwidgets/Types.h"

#include "mcc/ui/TextUtils.h"

#include <QFont>
#include <QPainter>
#include <QPixmap>

const QColor backgroundColorCommonValue(0, 0, 0, 153);
const QColor backgroundColorCommonHoveredValue(0, 0, 0, 198);

const QColor backgroundColorSelectedValue(0, 0, 220, 170);
const QColor backgroundColorSelectedHoveredValue(0, 0, 220, 220);

const QColor backgroundColorEditingValue(198, 132, 0, 153);
const QColor backgroundColorEditingHoveredValue(198, 132, 0, 220);

QString coloredTitle(const QString& channelName, bool activated)
{
    static const QString colorPrefix = "<a style=\"color:#%1\">";
    QString colorText;
    if(activated)
        colorText = colorPrefix.arg("00FF00");
    else
        colorText = colorPrefix.arg("B0B0B0");

    return QString(colorText + mccui::shortTextLine(channelName) + QString("</a>"));
}

QString coloredLabelText(const QString& name, uint color)
{
    static const QString colorPrefix = "<a style=\"color:#%1\">%2</a>";
    return colorPrefix.arg(color, 6, 16, QChar('0')).arg(name);
}

QPixmap scrollAmountPixmap(const QPixmap& original, int value)
{
    if(original.isNull())
        return original;

    QString text("∞");
    if(value > 0 && value < 100)
        text = QString::number(value);

    QImage img = original.toImage();
    QPainter painter(&img);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const int diameter = 14;
    QRect ellipse(img.width() - diameter, img.height() - diameter, diameter, diameter);
    painter.setPen(QColor(0, 0, 0, 0));
    painter.setBrush(QColor(200, 200, 200, 200));
    painter.drawEllipse(ellipse);

    painter.setPen(QColor(0, 0, 0, 200));
    QFont font;
    font.setFamily("Roboto Condensed");
    font.setStyleName("Bold");
    font.setPixelSize(10);
    painter.setFont(font);

    painter.drawText(ellipse, Qt::AlignCenter, text);

    return QPixmap::fromImage(img);
}

const QColor& backgroundColorCommon(bool hovered)
{
    if(hovered)
        return backgroundColorCommonHoveredValue;
    return backgroundColorCommonValue;
}

const QColor& backgroundColorSelected(bool hovered)
{
    if(hovered)
        return backgroundColorSelectedHoveredValue;
    return backgroundColorSelectedValue;
}

const QColor&backgroundColorEditing(bool hovered)
{
    if(hovered)
        return backgroundColorEditingHoveredValue;
    return backgroundColorEditingValue;
}
