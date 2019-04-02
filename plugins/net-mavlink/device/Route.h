#pragma once

namespace mccmav {

struct MavlinkPoint : mccgeo::Position
{
    uint16_t command = 0;
    MavlinkPoint(double latitude, double longitude, double altitude, uint16_t command)
        : mccgeo::Position(latitude, longitude, altitude), command(command)
    {

    }
};

class Route
{
public:

    explicit Route(std::vector<MavlinkPoint> &&waypoints = std::vector<MavlinkPoint>())
            : _nextWaypoint(0), _waypoints(std::move(waypoints))
    { }

    void clear();

    void reserve(std::size_t size);

    std::size_t size() const;

    void push_back(MavlinkPoint &&point);

    std::size_t nextWaypoint() const;

    void setNextWaypoint(std::size_t nextWaypoint);

    const std::vector<MavlinkPoint> &points() const;

    const MavlinkPoint &at(std::size_t index) const;

    void resize(size_t newSize);

    bool isEmpty() const;

private:

    std::size_t _nextWaypoint;
    std::vector<MavlinkPoint> _waypoints;

};

inline void Route::clear()
{
    _waypoints.clear();
    _nextWaypoint = 0;
}

inline void Route::reserve(std::size_t size)
{
    _waypoints.reserve(size);
}

inline std::size_t Route::size() const
{
    return _waypoints.size();
}

inline void Route::push_back(MavlinkPoint &&point)
{
    _waypoints.push_back(point);
}

inline std::size_t Route::nextWaypoint() const
{
    return _nextWaypoint;
}

inline void Route::setNextWaypoint(std::size_t nextWaypoint)
{
    //BMCL_ASSERT(nextWaypoint == 0 || nextWaypoint < size());
    _nextWaypoint = nextWaypoint;
}

inline const std::vector<MavlinkPoint> &Route::points() const
{
    return _waypoints;
}

inline const MavlinkPoint &Route::at(std::size_t index) const
{
    return _waypoints.at(index);
}

inline bool Route::isEmpty() const
{
    return _waypoints.empty();
}

inline void Route::resize(size_t newSize)
{
    BMCL_ASSERT(newSize <= _waypoints.size());
    std::size_t eraseCount = _waypoints.size() - newSize;
    _nextWaypoint = _nextWaypoint > 0 ? _nextWaypoint - eraseCount : 0;
    _waypoints.erase(_waypoints.begin(), _waypoints.begin() + eraseCount);
}
}
