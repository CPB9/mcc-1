#include "mcc/msg/Route.h"
#include "mcc/msg/ProtocolController.h"
#include <bmcl/Buffer.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

namespace mccmsg {


Waypoint::Waypoint()
    : isActive(false)
{
}

Waypoint::Waypoint(const mccgeo::Position& position,
    double speed,
    double direction,
    const PropertyValues& properties)
    : position(position)
    , speed(speed)
    , direction(direction)
    , properties(properties)
    , isActive(false)
{}

Waypoint::Waypoint(const mccgeo::Position& position,
    double speed,
    double direction,
    PropertyValues&& properties)
    : position(position)
    , speed(speed)
    , direction(direction)
    , properties(std::move(properties))
    , isActive(false)
{}

Waypoint::Waypoint(const mccgeo::Position& position,
    double speed,
    double direction)
    : position(position)
    , speed(speed)
    , direction(direction)
    , isActive(false)
{}

Waypoint::Waypoint(const mccgeo::Position& position, PropertyValues&& properties) : position(position), properties(std::move(properties)) {}


bool Waypoint::operator==(const Waypoint& other) const
{
    return (position == other.position) && (properties == other.properties);
}

bool Waypoint::operator!=(const Waypoint& other) const
{
    return !(*this == other);
}

BitFlags::BitFlags() {}
BitFlags::BitFlags(RouteFlag f) { set(f); }
BitFlags::BitFlags(std::initializer_list<RouteFlag> l){ set(l);}
BitFlags::~BitFlags() {}
bool BitFlags::hasSome() const { return _set.any(); }
bool BitFlags::get(RouteFlag f) const { return _set.test((uint8_t)f); }
void BitFlags::set(RouteFlag f, bool value) { _set.set((uint8_t)f, value); }
void BitFlags::set(std::initializer_list<RouteFlag> l)
{
    for (auto i : l)
    {
        _set.set((uint8_t)i, true);
    }
}
void BitFlags::reset(RouteFlag f) { _set.reset((uint8_t)f); }
void BitFlags::reset(std::initializer_list<RouteFlag> l)
{
    for (auto i : l)
    {
        _set.reset((uint8_t)i);
    }
}

RouteProperties::RouteProperties(std::size_t maxWaypoints, RouteName name, const std::string& info, BitFlags flags, bmcl::Option<std::size_t> nextWaypoint)
    : maxWaypoints(maxWaypoints), name(name), info(info), flags(flags), nextWaypoint(nextWaypoint)
{
}
RouteProperties::RouteProperties(std::size_t maxWaypoints, RouteName name, const std::string& info, bmcl::Option<std::size_t> nextWaypoint)
    : maxWaypoints(maxWaypoints), name(name), info(info), nextWaypoint(nextWaypoint)
{
}

Route::Route(const Waypoints& waypoints, const RouteProperties& properties) : waypoints(waypoints), properties(properties) { }
Route::Route(Waypoints&& waypoints, RouteProperties&& properties) : waypoints(std::move(waypoints)), properties(std::move(properties)) { }

TmRoute::TmRoute(const Device& device, const Route& route) : TmAny(device), _route(route) {}
TmRoute::TmRoute(const Device& device, Route&& route) : TmAny(device), _route(std::move(route)) {}
const Route& TmRoute::route() const { return _route; }

TmRoutesList::TmRoutesList(const Device& device, const RoutesProperties& properties, bmcl::Option<RouteName> activeRoute)
    : TmAny(device), _properties(properties), _activeRoute(activeRoute)
{
}
TmRoutesList::TmRoutesList(const Device& device, RoutesProperties&& properties, bmcl::Option<RouteName> activeRoute)
    : TmAny(device), _properties(std::move(properties)), _activeRoute(activeRoute)
{
}
const RoutesProperties& TmRoutesList::properties() const { return _properties; }
bmcl::Option<RouteName> TmRoutesList::activeRoute() const { return _activeRoute; }

bmcl::Buffer Route::encode() const
{
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

    writer.StartObject();

    {
        writer.Key("maxWaypoints");
        writer.Uint(properties.maxWaypoints);
        writer.Key("name");
        writer.Uint(properties.name);
        writer.Key("info");
        writer.String(properties.info);
        writer.Key("nextWaypoint");
        if (properties.nextWaypoint.isSome())
            writer.Uint(properties.nextWaypoint.unwrap());
        else
            writer.Null();
    }

    {
        writer.Key("flags");
        writer.StartObject();
        writer.Key("Hidden");
        writer.Bool(properties.flags.get(RouteFlag::Hidden));
        writer.Key("Locked");
        writer.Bool(properties.flags.get(RouteFlag::Locked));
        writer.Key("Closed");
        writer.Bool(properties.flags.get(RouteFlag::Closed));
        writer.Key("ReadOnly");
        writer.Bool(properties.flags.get(RouteFlag::ReadOnly));
        writer.Key("Inverted");
        writer.Bool(properties.flags.get(RouteFlag::Inverted));
        writer.Key("PointsOnly");
        writer.Bool(properties.flags.get(RouteFlag::PointsOnly));
        writer.EndObject();
    }

    {
        writer.Key("waypoints");
        writer.StartObject();
        for (const auto& i : waypoints)
        {
            writer.StartObject();
            writer.Key("lat");
            writer.Double(i.position.latitude());
            writer.Key("lon");
            writer.Double(i.position.longitude());
            writer.Key("alt");
            writer.Double(i.position.altitude());

            writer.Key("properties");
            writer.StartObject();
            for (const auto& j : i.properties.values())
            {
                writer.StartObject();
                writer.Key("name");
                writer.String(j->property()->name().toStdString());
                writer.Key("info");
                writer.String(j->property()->info());
                writer.Key("value");
                writer.String(j->encode());
                writer.EndObject();
            }
            writer.EndObject();

            writer.EndObject();
        }
        writer.EndObject();
    }

    writer.EndObject();

    bmcl::Buffer tmp;
    tmp.write(buffer.GetString(), buffer.GetSize());
    return tmp;
}

bmcl::Option<Route> Route::decode(const ProtocolController* pc, bmcl::Bytes bytes)
{
    rapidjson::Document d;
    d.Parse((const char*)bytes.data(), bytes.size());
    if (d.HasParseError() || !d.IsObject())
        return bmcl::None;

    const auto& points = d["waypoints"].GetArray();
    Waypoints waypoints;
    for (const auto& point : points)
    {
        mccgeo::Position pos(point["lat"].GetDouble(), point["lon"].GetDouble(), point["alt"].GetDouble());
        const auto properties = point["points"].GetArray();
        PropertyValues pvs;
        for (const auto& property : properties)
        {
            auto p = pc->decodeProperty(mccmsg::Property::createOrNil(property["name"].GetString()), property["value"].GetString());
            if (p.isNone())
                continue;
            pvs.add(p.take());
        }
        waypoints.emplace_back(Waypoint(pos, std::move(pvs)));
    }

    bmcl::Option<std::size_t> nextWaypoint;
    if (d.HasMember("nextWaypoint"))
        nextWaypoint = d["nextWaypoint"].GetUint();

    return Route(std::move(waypoints), RouteProperties(d["maxWaypoints"].GetUint(), d["name"].GetUint(),  d["info"].GetString(), nextWaypoint));
}

}
