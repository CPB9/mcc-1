#include "mcc/calib/IndicatorWidget.h"

#include <QPixmap>

namespace mcccalib {

IndicatorWidget::IndicatorWidget(QWidget *parent, const QString& vehiclePixmap, const QString& rotatedPixmap)
    : QLabel(parent), _rotated(false), _normalPixmap(vehiclePixmap), _rotatedPixmap(rotatedPixmap)
{
    setVisible(false);

    setScaledContents(true);
    QSizePolicy policy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    policy.setHeightForWidth(true);
    this->setSizePolicy(policy);

    setFixedSize(128, 128);
    setStatus(mccmsg::CalibrationSideStatus());
}

int IndicatorWidget::heightForWidth(int width) const
{
    if (width > pixmap()->width()) {
        return pixmap()->height();
    }
    else {
        return ((qreal)pixmap()->height()*width) / pixmap()->width();
    }
}

void IndicatorWidget::setStatus(const mccmsg::CalibrationSideStatus& status)
{
    QString color = "red";
    if (status.inProgress)
        color = "yellow";
    if (status.done)
        color = "green";

    setVisible(status.visible);
    if (status.visible && !pixmap())
    {
        setPixmap(_normalPixmap);
    }

    if (status.rotate && !_rotated)
    {
        setPixmap(QPixmap(_rotatedPixmap));
        _rotated = true;
    }

    if (!status.rotate && _rotated)
    {
        setPixmap(QPixmap(_normalPixmap));
        _rotated = false;
    }

    setStyleSheet("border: 5px solid " + color);
}
}
