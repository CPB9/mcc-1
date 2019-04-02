#pragma once

#include <QPen>

#include <memory>

namespace mccuav {

struct WaypointStyle {
    QPen linePen;
    QColor activePointColor;
    QColor inactivePointColor;
    QColor hoverColor;
    QColor selectionColor;
    bool hasDetails;
};

enum class RouteState
{
    UnderEdit,
    Inactive,
    Active,
    Selected,
};

struct RouteStyle {
    WaypointStyle active;
    WaypointStyle inactive;
    WaypointStyle editable;

    void setState(RouteState state)
    {
        _state = state;
        switch (state)
        {
        case RouteState::UnderEdit:
            active.linePen.setWidth(1);
            active.hasDetails = false;
            break;
        case RouteState::Inactive:
            active.linePen.setWidth(1);
            active.hasDetails = true;
            break;
        case RouteState::Active:
            active.linePen.setWidth(3);
            active.hasDetails = true;
            break;
        case RouteState::Selected:
            active.linePen.setWidth(5);
            active.hasDetails = true;
            break;
        }
    }

    void setLineColor(const QColor& color)
    {
        active.linePen.setColor(color);
    }

    RouteState _state;
};

typedef std::shared_ptr<RouteStyle> RouteStylePtr;
}
