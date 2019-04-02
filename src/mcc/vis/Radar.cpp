#include "mcc/vis/Radar.h"
#include "mcc/hm/HmReader.h"
#include "mcc/geo/Constants.h"

#include <bmcl/Logging.h>
#include <bmcl/Buffer.h>
#include <bmcl/MemReader.h>
#include <bmcl/Result.h>

#include <QFile>

namespace mccvis {

Radar::Radar()
    : _isVisible(false)
    , _isModified(false)
{
}

Radar::~Radar()
{
}

Radar::Radar(const mccgeo::LatLon& position)
    : _position(position)
    , _isVisible(false)
    , _isModified(false)
{
}

Radar::Radar(const mccgeo::LatLon& position, const ViewParams& params)
    : _position(position)
    , _params(params)
    , _isVisible(false)
    , _isModified(false)
{
}

bool Radar::isVisible() const
{
    return _isVisible;
}


const QString& Radar::name() const
{
    return _name;
}

void Radar::setVisibility(bool isVisible)
{
    _isModified = true;
    _isVisible = isVisible;
}

const ViewParams& Radar::viewParams() const
{
    return _params;
}

void Radar::setParams(const ViewParams& params)
{
    _isModified = true;
    _params = params;
    _name = QString::fromStdString(params.name);
}

void Radar::setPosition(const mccgeo::LatLon& position)
{
    _isModified = true;
    _position = position;
}

std::vector<Point> Radar::visionArea(const mcchm::HmReader* handler, const mccgeo::Geod& geod, const std::vector<mccgeo::PositionAndDistance>& profile) const {

    std::vector<mccvis::Point> top;
    std::vector<mccvis::Point> bot;

    for (const mccgeo::PositionAndDistance& pAd : profile) {
        const mccgeo::Position& point = pAd.position();
        double d = 0;
        double a1 = 0;
        double a2 = 0;
        geod.inverse(_position, point.latLon(), &d, &a1, &a2);
        if (d > _params.maxBeamDistance || d < _params.minBeamDistance) {
            continue;
        }
        double a1norm = a1;
        while (a1norm < _params.minAzimuth) {
            a1norm += 360;
        }
        if (!(a1norm >= _params.minAzimuth && a1norm <= _params.maxAzimuth)) {
            continue;
        }
        mccgeo::LatLon newEnd;
        geod.direct(_position, a1, d, &newEnd, &a2);

        std::vector<Point> slice;
        if (_params.useCalcStep) {
            double step = std::max(90.0, _params.calcStep);
            slice = handler->relativePointProfile(_position, newEnd, step);
        } else {
            slice = handler->relativePointProfileAutostep(_position, newEnd);
        }

        slice.push_back(mccvis::Point(d, point.altitude()));
        Profile vI(a1norm, slice, viewParams());
        bmcl::Option<std::pair<double, double>> interval = vI.verticalVisionIntervalAt(d);

        if (interval.isSome()) {
            double botP = interval->first;
            double topP = interval->second;
            bot.emplace_back(pAd.distance() / 1000, botP);
            top.emplace_back(pAd.distance() / 1000, topP);
        }
    }
    std::reverse(top.begin(), top.end());
    bot.insert(bot.end(), top.begin(), top.end());
    return bot;
}

const mccgeo::LatLon& Radar::position() const
{
    return _position;
}

#define PARAMS_SIZE 8 * 18 + 4 * 2 + 1 * 10
#define RADAR_VER 7

template <typename T>
static void writeDoubleLe(T* w, double value)
{
    uint64_t data;
    assert(sizeof(value) == sizeof(data));
    std::memcpy(&data, &value, sizeof(value));
    w->writeUint64Le(data);
}

static double readDoubleLe(bmcl::MemReader* r)
{
    uint64_t data = r->readUint64Le();
    double value;
    assert(sizeof(value) == sizeof(data));
    std::memcpy(&value, &data, sizeof(value));
    return value;
}

void Radar::serialize(bmcl::Buffer* dest) const
{
    const Radar* radar = this;
    const std::string& nameUtf8 = radar->_params.name;
    QByteArray pathUtf8;
    if (_savePath.isSome()) {
        pathUtf8 = radar->_savePath.unwrap().toUtf8();
    }

    constexpr std::size_t verSize = sizeof(uint32_t);
    constexpr std::size_t headerSize = sizeof(uint32_t);
    constexpr std::size_t latLonSize = sizeof(double) * 2;
    constexpr std::size_t paramsSize = PARAMS_SIZE;
    constexpr std::size_t stringHeaderSize = sizeof(uint32_t);
    std::size_t stringSize = nameUtf8.size();
    std::size_t dataSize =  stringHeaderSize + stringSize + latLonSize + paramsSize + 1 + 1 + 1;
    if (_savePath.isSome()) {
        dataSize += 4 + pathUtf8.size();
    }
    std::size_t size = verSize + headerSize + dataSize;

    dest->writeUint32Le(RADAR_VER);
    dest->reserve(dest->size() + size);
    dest->writeUint32Le((uint32_t)dataSize);
    dest->writeUint32Le((uint32_t)stringSize);
    dest->write(nameUtf8.data(), stringSize);
    writeDoubleLe(dest, radar->position().latitude());
    writeDoubleLe(dest, radar->position().longitude());
    writeDoubleLe(dest, radar->viewParams().minBeamDistance);
    writeDoubleLe(dest, radar->viewParams().maxBeamDistance);
    writeDoubleLe(dest, radar->viewParams().minAzimuth);
    writeDoubleLe(dest, radar->viewParams().maxAzimuth);
    writeDoubleLe(dest, radar->viewParams().radarHeight);
    writeDoubleLe(dest, radar->viewParams().minAngle);
    writeDoubleLe(dest, radar->viewParams().maxAngle);
    writeDoubleLe(dest, radar->viewParams().deltaAngle);
    writeDoubleLe(dest, radar->viewParams().angleStep);
    writeDoubleLe(dest, radar->viewParams().frequency);
    writeDoubleLe(dest, radar->viewParams().calcStep);
    writeDoubleLe(dest, radar->viewParams().objectHeight);
    writeDoubleLe(dest, radar->viewParams().targetSpeed);
    writeDoubleLe(dest, radar->viewParams().missleSpeed);
    writeDoubleLe(dest, radar->viewParams().minHitDistance);
    writeDoubleLe(dest, radar->viewParams().maxHitDistance);
    writeDoubleLe(dest, radar->viewParams().reactionTime);
    writeDoubleLe(dest, radar->viewParams().externReactionTime);
    dest->writeUint32Le(radar->viewParams().viewZonesColorArgb);
    dest->writeUint32Le(radar->viewParams().hitZonesColorArgb);
    dest->writeUint8(radar->viewParams().isRelativeHeight);
    dest->writeUint8(radar->viewParams().isBidirectional);
    dest->writeUint8(radar->viewParams().useFresnelRegion);
    dest->writeUint8(radar->viewParams().useCalcStep);
    dest->writeUint8(radar->viewParams().isTargetRelativeHeight);
    dest->writeUint8(radar->viewParams().isTargetDirectedTowards);
    dest->writeUint8(radar->viewParams().calcHits);
    dest->writeUint8(radar->viewParams().canViewGround);
    dest->writeUint8(radar->viewParams().hasRefraction);
    dest->writeUint8(radar->viewParams().additionalDistancePercent);
    dest->writeUint8(radar->isVisible());
    dest->writeUint8(radar->isModified());
    if (_savePath.isSome()) {
        dest->writeUint8(1);
        dest->writeUint32Le(pathUtf8.size());
        dest->write(pathUtf8.data(), pathUtf8.size());
    } else {
        dest->writeUint8(0);
    }
}

bmcl::Result<Rc<Radar>, void> Radar::deserialize(bmcl::MemReader* src)
{
    mccgeo::LatLon position;
    QString name;
    ViewParams params;
    bool isEnabled = false;
    bool hasSavePath = false;
    bool isModified = false;
    if (src->readableSize() < 4) {
        return bmcl::Result<Rc<Radar>, void>();
    }
    uint32_t version = src->readUint32Le();
    if (version != RADAR_VER) {
        return bmcl::Result<Rc<Radar>, void>();
    }
    if (src->readableSize() < 4) {
        return bmcl::Result<Rc<Radar>, void>();
    }
    uint32_t nextSize = src->readUint32Le();
    if (nextSize > src->readableSize()) {
        return bmcl::Result<Rc<Radar>, void>();
    }
    bmcl::MemReader current(src->current(), nextSize);
    src->skip(nextSize);
    if (current.readableSize() < 4) {
        return bmcl::Result<Rc<Radar>, void>();
    }
    uint32_t stringSize = current.readUint32Le();
    if (current.readableSize() < stringSize) {
        return bmcl::Result<Rc<Radar>, void>();
    }
    std::string nameUtf8((char*)current.current(), stringSize);
    current.skip(stringSize);
    name = QString::fromUtf8(nameUtf8.c_str(), nameUtf8.size());
    if (current.readableSize() < (8 * 2 + PARAMS_SIZE + 1 + 1 + 1)) {
        return bmcl::Result<Rc<Radar>, void>();
    }
    position.latitude() = readDoubleLe(&current);
    position.longitude() = readDoubleLe(&current);

    params.minBeamDistance = readDoubleLe(&current);
    params.maxBeamDistance = readDoubleLe(&current);
    params.minAzimuth = readDoubleLe(&current);
    params.maxAzimuth = readDoubleLe(&current);
    params.radarHeight = readDoubleLe(&current);
    params.minAngle = readDoubleLe(&current);
    params.maxAngle = readDoubleLe(&current);
    params.deltaAngle = readDoubleLe(&current);
    params.angleStep = readDoubleLe(&current);
    params.frequency = readDoubleLe(&current);
    params.calcStep = readDoubleLe(&current);
    params.objectHeight = readDoubleLe(&current);
    params.targetSpeed = readDoubleLe(&current);
    params.missleSpeed = readDoubleLe(&current);
    params.minHitDistance = readDoubleLe(&current);
    params.maxHitDistance = readDoubleLe(&current);
    params.reactionTime = readDoubleLe(&current);
    params.externReactionTime = readDoubleLe(&current);
    params.viewZonesColorArgb = current.readUint32Le();
    params.hitZonesColorArgb = current.readUint32Le();
    params.isRelativeHeight = current.readUint8();
    params.isBidirectional = current.readUint8();
    params.useFresnelRegion = current.readUint8();
    params.useCalcStep = current.readUint8();
    params.isTargetRelativeHeight = current.readUint8();
    params.isTargetDirectedTowards = current.readUint8();
    params.calcHits = current.readUint8();
    params.canViewGround = current.readUint8();
    params.hasRefraction = current.readUint8();
    params.additionalDistancePercent = current.readUint8();
    params.name = nameUtf8;

    isEnabled = current.readUint8();
    isModified = current.readUint8();
    hasSavePath = current.readUint8();

    RadarPtr radar = new Radar(position, params);
    radar->setVisibility(isEnabled);

    if (hasSavePath) {
        if (current.readableSize() < 4) {
            return bmcl::Result<Rc<Radar>, void>();
        }
        uint32_t pathLen = current.readUint32Le();
        if (current.readableSize() < pathLen) {
            return bmcl::Result<Rc<Radar>, void>();
        }
        QByteArray pathUtf8((char*)current.current(), pathLen);
        current.skip(pathLen);
        QString path = QString::fromUtf8(pathUtf8);
        radar->_savePath.emplace(path);
    }

    radar->_isModified = isModified;
    radar->_name = name;
    return radar;
}

bool Radar::saveAs(const QString& path)
{
    _savePath = path;
    return save();
}

bool Radar::save()
{
    if (_savePath.isNone()) {
        return false;
    }
    bmcl::Buffer dest;
    serialize(&dest);

    QFile file(_savePath.unwrap());
    if (!file.open(QFile::WriteOnly)) {
        return false;
    }

    qint64 rv = file.write((const char*)dest.data(), dest.size());
    if (rv != dest.size()) {
        return false;
    }

    _isModified = false;
    return true;
}

bmcl::Result<Rc<Radar>, void> Radar::loadFrom(const QString& path)
{
    QFile file(path);
    if (!file.open(QFile::ReadOnly)) {
        return bmcl::Result<Rc<Radar>, void>();
    }

    QByteArray data = file.readAll();
    bmcl::MemReader src(data.data(), data.size());
    auto rv = deserialize(&src);
    if (rv.isErr()) {
        return bmcl::Result<Rc<Radar>, void>();
    }
    rv.unwrap()->_savePath.emplace(path);
    return rv;
}

const bmcl::Option<QString>& Radar::savePath() const
{
    return _savePath;
}

bool Radar::isModified() const
{
    return _isModified;
}

void Radar::setSavePath(const bmcl::Option<QString>& path)
{
    _savePath = path;
}

void Radar::setModified(bool isModified)
{
    _isModified = isModified;
}
}
