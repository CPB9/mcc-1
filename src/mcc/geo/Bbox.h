#pragma once

#include "mcc/geo/Config.h"
#include "mcc/geo/LatLon.h"

namespace mccgeo {

class MCC_GEO_DECLSPEC Bbox {
public:
    Bbox();
    explicit Bbox(const LatLon& single);
    Bbox(const LatLon& topLeft, const LatLon& bottomRight);

    bool isPoint() const; //FIXME: doubleEq

    const LatLon& topLeft() const;
    LatLon topRight() const;
    const LatLon& bottomRight() const;
    LatLon bottomLeft() const;
    LatLon center() const;
    double left() const;
    double right() const;
    double top() const;
    double bottom() const;

    void setTopLeft(const LatLon& topLeft);
    void setBottomRight(const LatLon& bottomRight);
    void setLeft(double left);
    void setRight(double right);
    void setTop(double top);
    void setBottom(double bottom);

private:
    LatLon _topLeft;
    LatLon _bottomRight;
};
}
