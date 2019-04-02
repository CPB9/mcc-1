#pragma once

#include <QLabel>
#include <QPixmap>

#include "mcc/msg/Calibration.h"

namespace mcccalib {

class IndicatorWidget : public QLabel
{
    Q_OBJECT
public:
    IndicatorWidget(QWidget *parent, const QString& vehiclePixmap, const QString& rotatedPixmap);

    virtual int heightForWidth(int width) const override;
    virtual bool hasHeightForWidth() const override { return true; }
    virtual QSize sizeHint() const override { return pixmap()->size(); }
    virtual QSize minimumSizeHint() const override { return QSize(0, 0); }

    void setStatus(const mccmsg::CalibrationSideStatus& status);
private:
    bool _rotated;
    QString _normalPixmap;
    QString _rotatedPixmap;
};
}
