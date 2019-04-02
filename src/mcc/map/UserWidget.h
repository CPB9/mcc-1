#pragma once

#include "mcc/Config.h"

#include <QWidget>

namespace mccmap {

class MapWidget;

class MCC_MAP_DECLSPEC UserWidget : public QWidget
{
    Q_OBJECT

public:
    explicit UserWidget(QWidget* parent);
    ~UserWidget() override;

    virtual void setMapWidget(MapWidget *map);

public slots:
    virtual void adjustPosition() {}

protected:
    MapWidget *mapWidget() {return _mapWidget;}

private:
    MapWidget *_mapWidget;

    Q_DISABLE_COPY(UserWidget)
};
}
