#include "mcc/map/UserWidget.h"
#include "mcc/map/MapWidget.h"

#include <QApplication>

namespace mccmap {

UserWidget::UserWidget(QWidget* parent)
    : QWidget(parent)
    , _mapWidget(nullptr)
{}

void UserWidget::setMapWidget(MapWidget* map)
{
    if(map == _mapWidget || map == nullptr)
        return;

    _mapWidget = map;
    setParent(map);
}

UserWidget::~UserWidget()
{}

}
