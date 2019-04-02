#pragma once

#include "mcc/Config.h"
#include "mcc/geo/Fwd.h"

#include <bmcl/Fwd.h>

#include <QObject>

namespace mccui {

class MCC_UI_DECLSPEC Trackable : public QObject {
    Q_OBJECT
public:
    virtual bmcl::Option<mccgeo::LatLon> position() const;

signals:
    void positionUpdated(const mccgeo::LatLon& position);
    void trackingStopped();
    void trackingEnabled(bool isEnabled);
};
}
