#include "mcc/geo/EnuPositionHandler.h"
#include "mcc/geo/GroupGeometry.h"
#include <cmath>

namespace mccgeo {

EnuPositionHandler::EnuPositionHandler() : _lastMoment(0), _lastGravityCenterPosition(Vector3D()){

}

EnuPositionHandler::~EnuPositionHandler() {}

Vector3D EnuPositionHandler::toEnuPosition(const Position &pos, const Position &p0) {
    return pos.toEnu(p0);
}

std::vector<Vector3D> EnuPositionHandler::toEnuPosition(const std::vector<Position>& ps, const Position& p0)
{
    std::vector<Vector3D> result;
    result.reserve(ps.size());
    for (auto& p : ps) {
        result.emplace_back(toEnuPosition(p, p0));
    }
    return result;
}

Position EnuPositionHandler::toGpsPosition(const Vector3D& p, const Position& gps0) {
    return p.toPositionFromEnu(gps0);
}

std::vector<Position> EnuPositionHandler::toGpsPosition(const std::vector<Vector3D>& ps, const Position& gps0) {
    std::vector<Position> result;
    result.reserve(ps.size());
    for (auto& pos : ps) {
        result.emplace_back(toGpsPosition(pos, gps0));
    }

    return result;
}

GroupGeometry EnuPositionHandler::calcGroupParams(const std::vector<Position>& poss, double time) {
    if (poss.size() == 0)
        return GroupGeometry(Position(), Vector3D(), 0, 0, 0, 0);

    std::vector<Vector3D> enuPoss = toEnuPosition(poss, poss.at(0));

    double cx = 0;
    double cy = 0;
    double cz = 0;
    int size = enuPoss.size();
    for (auto& pos : enuPoss) {
        cx += pos.x();
        cy += pos.y();
        cz += pos.z();
    }
    Vector3D enuCenter(cx/size, cy/size, cz/size);
    Position gpsCenter = toGpsPosition(enuCenter, poss.at(0));

    Vector3D velCenter;
    if (std::isnormal(_lastMoment) && !_lastGravityCenterPosition.isNull()) {
        double dt = time - _lastMoment;
        Vector3D dP = enuCenter - _lastGravityCenterPosition;
        if (dt > 0)
            velCenter = dP / dt;
    }
    _lastGravityCenterPosition = enuCenter;
    _lastMoment = time;

    double maxRem = 0;
    for (auto& pos : enuPoss) {
        Vector3D r = enuCenter - pos;
        double rem = r.length();
        if (rem > maxRem)
            maxRem = rem;
    }

    double minConv = std::numeric_limits<double>::max();
    int index1 = 0;
    int index2 = 0;
    int k1 = 0;
    for (auto& pos1 : enuPoss) {
        int k2 = 0;
        for (auto& pos2 : enuPoss) {
            if (pos1 == pos2)
                continue;
            Vector3D r = pos1 - pos2;
            double conv = r.length();
            if (conv < minConv) {
                minConv = conv;
                index1 = k1;
                index2 = k2;
            }
            k2++;
        }
        k1++;
    }

    return GroupGeometry(gpsCenter, velCenter, maxRem, minConv, index1, index2);
}

}
