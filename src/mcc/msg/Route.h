#pragma once
#include "mcc/Config.h"
#include <vector>
#include <bitset>
#include <bmcl/Option.h>
#include "mcc/msg/Tm.h"
#include "mcc/msg/Cmd.h"
#include "mcc/msg/Property.h"
#include "mcc/geo/Position.h"

namespace mccmsg {

struct MCC_MSG_DECLSPEC Waypoint
{
    Waypoint();
    Waypoint(const mccgeo::Position& position, double speed, double direction, const PropertyValues& properties);
    Waypoint(const mccgeo::Position& position, double speed, double direction, PropertyValues&& properties);
    Waypoint(const mccgeo::Position& position, PropertyValues&& properties);
    Waypoint(const mccgeo::Position& position, double speed, double direction);
    bool operator==(const Waypoint& other) const;
    bool operator!=(const Waypoint& other) const;

    mccgeo::Position position;
    bool isActive;
    double speed;
    double direction;
    PropertyValues properties;
};
using Waypoints = std::vector<Waypoint>;

enum class RouteFlag : uint8_t
{
    Hidden = 0, Locked = 1, Closed = 2, ReadOnly = 3, Inverted = 4, PointsOnly = 5
};

class MCC_MSG_DECLSPEC BitFlags
{
public:
    BitFlags();
    BitFlags(RouteFlag);
    BitFlags(std::initializer_list<RouteFlag> l);
    ~BitFlags();
    bool hasSome() const;
    bool get(RouteFlag) const;
    void set(RouteFlag, bool value = true);
    void set(std::initializer_list<RouteFlag> l);
    void reset(RouteFlag);
    void reset(std::initializer_list<RouteFlag> l);
private:
    std::bitset<8> _set;
};

using RouteName = uint32_t;
struct MCC_MSG_DECLSPEC RouteProperties
{
    RouteProperties(std::size_t maxWaypoints, RouteName name, const std::string& info, BitFlags flags, bmcl::Option<std::size_t> nextWaypoint = bmcl::None);
    RouteProperties(std::size_t maxWaypoints, RouteName name, const std::string& info, bmcl::Option<std::size_t> nextWaypoint = bmcl::None);
    std::size_t maxWaypoints;
    RouteName name;
    std::string info;
    BitFlags flags;
    bmcl::Option<std::size_t> nextWaypoint;
};
using RoutesProperties = std::vector<RouteProperties> ;

class MCC_MSG_DECLSPEC Route
{
public:
    Route(const Waypoints& waypoints, const RouteProperties& properties);
    Route(Waypoints&& waypoints, RouteProperties&& properties);
    bmcl::Buffer encode() const;
    static bmcl::Option<Route> decode(const ProtocolController*, bmcl::Bytes);

    Waypoints waypoints;
    RouteProperties properties;
};
using Routes = std::vector<Route>;

class MCC_MSG_DECLSPEC TmRoute : public TmAny
{
public:
    TmRoute(const Device& device, const Route& route);
    TmRoute(const Device& device, Route&& route);
    void visit(TmVisitor* visitor) const override;
    const Route& route() const;
private:
    Route _route;
};

class MCC_MSG_DECLSPEC TmRoutesList : public TmAny
{
public:
    TmRoutesList(const Device& device, const RoutesProperties& properties, bmcl::Option<RouteName> activeRoute = bmcl::None);
    TmRoutesList(const Device& device, RoutesProperties&& properties, bmcl::Option<RouteName> activeRoute = bmcl::None);
    void visit(TmVisitor* visitor) const override;
    const RoutesProperties& properties() const;
    const bmcl::Option<RouteName>& activeRoute() const;
private:
    RoutesProperties _properties;
    bmcl::Option<RouteName> _activeRoute;
};

class MCC_MSG_DECLSPEC CmdRouteSet : public DevReq
{
public:
    CmdRouteSet(const Device& device, const Route& route);
    CmdRouteSet(const Device& device, Route&& route);
    ~CmdRouteSet();
    void visit(CmdVisitor* visitor) const override;
    const char* nameXXX() const override;
    const char* info() const override;
    const Route& route() const;
private:
    Route _route;
};

class MCC_MSG_DECLSPEC CmdRouteSetNoActive : public DevReq
{
public:
    explicit CmdRouteSetNoActive(const Device& device);
    ~CmdRouteSetNoActive();
    void visit(CmdVisitor* visitor) const override;
    const char* nameXXX() const override;
    const char* info() const override;
};

class MCC_MSG_DECLSPEC CmdRouteSetActive : public DevReq
{
public:
    CmdRouteSetActive(const Device& device, uint32_t route, const bmcl::Option<uint32_t>& point);
    ~CmdRouteSetActive();
    void visit(CmdVisitor* visitor) const override;
    const char* nameXXX() const override;
    const char* info() const override;
    uint32_t route() const;
    const bmcl::Option<uint32_t>& point() const;
private:
    uint32_t _route;
    bmcl::Option<uint32_t> _point;
};

class MCC_MSG_DECLSPEC CmdRouteSetActivePoint : public DevReq
{
public:
    CmdRouteSetActivePoint(const Device& device, uint32_t route, const bmcl::Option<uint32_t>& point);
    ~CmdRouteSetActivePoint();
    void visit(CmdVisitor* visitor) const override;
    const char* nameXXX() const override;
    const char* info() const override;
    uint32_t route() const;
    const bmcl::Option<uint32_t>& point() const;
private:
    uint32_t _route;
    bmcl::Option<uint32_t> _point;
};

class MCC_MSG_DECLSPEC CmdRouteSetDirection : public DevReq
{
public:
    CmdRouteSetDirection(const Device& device, uint32_t route, bool isForward);
    ~CmdRouteSetDirection();
    void visit(CmdVisitor* visitor) const override;
    const char* nameXXX() const override;
    const char* info() const override;
    uint32_t route() const;
    bool isForward() const;
private:
    uint32_t _route;
    bool _isForward;
};

class MCC_MSG_DECLSPEC CmdRouteGetList : public DevReq
{
public:
    explicit CmdRouteGetList(const Device& device);
    ~CmdRouteGetList();
    void visit(CmdVisitor* visitor) const override;
    const char* nameXXX() const override;
    const char* info() const override;
};

class MCC_MSG_DECLSPEC CmdRouteGet : public DevReq
{
public:
    CmdRouteGet(const Device& device, uint32_t route);
    ~CmdRouteGet();
    void visit(CmdVisitor* visitor) const override;
    const char* nameXXX() const override;
    const char* info() const override;
    uint32_t route() const;
private:
    uint32_t _route;
};

class MCC_MSG_DECLSPEC CmdRouteCreate : public DevReq
{
public:
    CmdRouteCreate(const Device& device, uint32_t route);
    ~CmdRouteCreate();
    void visit(CmdVisitor* visitor) const override;
    const char* nameXXX() const override;
    const char* info() const override;
    uint32_t route() const;
private:
    uint32_t _route;
};

class MCC_MSG_DECLSPEC CmdRouteRemove : public DevReq
{
public:
    CmdRouteRemove(const Device& device, uint32_t route);
    ~CmdRouteRemove();
    void visit(CmdVisitor* visitor) const override;
    const char* nameXXX() const override;
    const char* info() const override;
    uint32_t route() const;
private:
    uint32_t _route;
};

class MCC_MSG_DECLSPEC CmdRouteClear : public DevReq
{
public:
    CmdRouteClear(const Device& device, uint32_t route);
    ~CmdRouteClear();
    void visit(CmdVisitor* visitor) const override;
    const char* nameXXX() const override;
    const char* info() const override;
    uint32_t route() const;
private:
    uint32_t _route;
};


}
