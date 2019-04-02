#include "mcc/hm/OmhmReader.h"

#include "mcc/geo/CoordinateConverter.h"
#include "mcc/geo/Coordinate.h"

#include <bmcl/Result.h>
#include <bmcl/MemReader.h>

#include <QString>
#include <QDebug>

namespace mcchm {

// universal_crc
// zlib crc-32

static const uint32_t crcTable[16] = {
    0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
    0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
    0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
    0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
};

static constexpr uint32_t crcInit(void)
{
    return 0xffffffff;
}

static inline uint32_t crcNext(uint32_t crc, uint8_t data)
{
    crc ^= data;
    crc = (crc >> 4) ^ crcTable[crc & 15];
    crc = (crc >> 4) ^ crcTable[crc & 15];
    return crc;
}

static constexpr uint32_t crcFinal(uint32_t crc)
{
    return ~crc;
}

std::uint32_t OmhmReader::crc32(const void* src, std::size_t len)
{
    const uint8_t* data = (const uint8_t*)src;
    uint32_t crc = crcInit();

    if (len) do {
        crc = crcNext(crc, *data++);
    } while (--len);

    return crcFinal(crc);
}

OmhmReader::OmhmReader(const RcGeod* wgs84Geod)
    : HmReader(wgs84Geod)
{
}

OmhmReader::~OmhmReader()
{
}

bmcl::Result<Rc<OmhmReader>, QString> OmhmReader::create(const RcGeod* wgs84Geod, const QString& filePath)
{
    Rc<OmhmReader> reader = new OmhmReader(wgs84Geod);
    auto err = reader->open(filePath);
    if (err.isSome()) {
        return err.take();
    }
    return std::move(reader);
}

bmcl::Option<QString> OmhmReader::open(const QString& filePath)
{
    _file.setFileName(filePath);

    if (!_file.open(QFile::ReadOnly)) {
        return _file.errorString();
    }

    _data = _file.map(0, _file.size());
    if (!_data) {
        return _file.errorString();
    }

    bmcl::MemReader reader(_data, _file.size());
    if (reader.size() < 6 * 4 + 8 * 10) {
        return QString("invalid file size");
    }

    uint32_t magic = reader.readUint32Le();
    if (magic != magicHeader) {
        return QString("invalid magic header");
    }

    uint32_t version = reader.readUint32Le();
    if (version != 1) {
        return QString("invalid version tag");
    }
    reader.readUint32Le(); //reserved

    _dtype = (OmhmDataType)reader.readUint32Le();
    switch (_dtype) {
    case OmhmDataType::Int8:
    case OmhmDataType::UInt8:
        _cellSize = 1;
        break;
    case OmhmDataType::Int16:
    case OmhmDataType::UInt16:
        _cellSize = 2;
        break;
    case OmhmDataType::Int32:
    case OmhmDataType::UInt32:
    case OmhmDataType::Float32:
        _cellSize = 4;
        break;
    case OmhmDataType::Int64:
    case OmhmDataType::UInt64:
    case OmhmDataType::Float64:
        _cellSize = 8;
        break;
    default:
        return QString("invalid data type tag");
    }

    _width = reader.readUint32Le();
    _height = reader.readUint32Le();
    _t0 = reader.readFloat64Le();
    _t1 = reader.readFloat64Le();
    _t2 = reader.readFloat64Le();
    _t3 = reader.readFloat64Le();
    _t4 = reader.readFloat64Le();
    _t5 = reader.readFloat64Le();
    _t2t4_t1t5 = _t2 * _t4 - _t1 * _t5;
    _noDataValue = reader.readFloat64Le();
    _scale = reader.readFloat64Le();
    _offset = reader.readFloat64Le();

    qDebug() << filePath << _t0 << _t1 << _t2 << _t3 << _t4 << _t5 << _noDataValue << _scale << _offset << _width << _height << (int)_dtype;

    std::uint64_t projDescSize = reader.readUint64Le();
    if (projDescSize > reader.sizeLeft()) {
        return QString("invalid description size");
    }

    const uint8_t* begin = reader.current();
    reader.skip(projDescSize);

    std::string desc((const char*)begin, (const char*)reader.current());

    if (reader.sizeLeft() < 4) {
        return QString("invalid file size");
    }

    const uint8_t* headerEnd = reader.current();

    std::uint32_t crc = reader.readUint32Le();

    if (crc32(_data, headerEnd - _data) != crc) {
        return QString("invalid header crc");
    }

    auto conv = mccgeo::CoordinateConverter::createFromProj4Definition(desc.c_str());
    if (conv.isErr()) {
        return QString::fromStdString(conv.unwrapErr());
    }
    _conv = conv.take();

    auto converted = _conv->convertInverse(mccgeo::Coordinate(_t0, _t3, 0, 0));
    qDebug() << converted.x() << converted.y();

    _data = reader.current();

    if (std::size_t(_cellSize) * _width * _height != reader.sizeLeft()) {
        return QString("invalid file size data section");
    }

    return bmcl::None;
}

template <typename T>
inline T readData(const uint8_t* ptr);

template <>
inline std::int8_t readData(const uint8_t* ptr)
{
    return *(std::int8_t*)ptr;
}

template <>
inline std::uint8_t readData(const uint8_t* ptr)
{
    return *ptr;
}

template <>
inline std::int16_t readData(const uint8_t* ptr)
{
    return le16dec(ptr);
}

template <>
inline std::uint16_t readData(const uint8_t* ptr)
{
    return le16dec(ptr);
}

template <>
inline std::int32_t readData(const uint8_t* ptr)
{
    return le32dec(ptr);
}

template <>
inline std::uint32_t readData(const uint8_t* ptr)
{
    return le32dec(ptr);
}

template <>
inline std::int64_t readData(const uint8_t* ptr)
{
    return le64dec(ptr);
}

template <>
inline std::uint64_t readData(const uint8_t* ptr)
{
    return le64dec(ptr);
}

template <>
inline float readData(const uint8_t* ptr)
{
    union {
        float value;
        std::uint32_t data;
    } data;
    data.data = le32dec(ptr);
    return data.value;
}

template <>
inline double readData(const uint8_t* ptr)
{
    union {
        double value;
        std::uint64_t data;
    } data;
    data.data = le64dec(ptr);
    return data.value;
}

template <typename T>
Altitude OmhmReader::readAndScaleData(std::size_t offset) const
{
    const uint8_t* ptr = _data + offset;
    double data = readData<T>(ptr);
    if (data == _noDataValue) {
        return bmcl::None;
    }

    return std::fma(data, _scale, _offset);
}

Altitude OmhmReader::readAltitude(mccgeo::LatLon latLon, double prec) const
{
    mccgeo::Coordinate converted = _conv->convertForward(latLon);

    double x = converted.x();
    double y = converted.y();

    if (!std::isfinite(x) || !std::isfinite(y)) {
        return bmcl::None;
    }

    double t3_y = _t3 - y;
    double x_t0 = x - _t0;
    double p = -(_t2 * t3_y + _t5 * x_t0) / _t2t4_t1t5;
    double l = (_t1 * t3_y + _t4 * x_t0) / _t2t4_t1t5;

    if (p < 0 || p > _width || l < 0 || l > _height) {
        return bmcl::None;
    }

    std::size_t offset = (std::size_t(l) * _width + std::size_t(p)) * _cellSize;

    switch (_dtype) {
    case OmhmDataType::Int8:
        return readAndScaleData<std::int8_t>(offset);
    case OmhmDataType::Int16:
        return readAndScaleData<std::int16_t>(offset);
    case OmhmDataType::Int32:
        return readAndScaleData<std::int32_t>(offset);
    case OmhmDataType::Int64:
        return readAndScaleData<std::int64_t>(offset);
    case OmhmDataType::UInt8:
        return readAndScaleData<std::uint8_t>(offset);
    case OmhmDataType::UInt16:
        return readAndScaleData<std::uint16_t>(offset);
    case OmhmDataType::UInt32:
        return readAndScaleData<std::uint32_t>(offset);
    case OmhmDataType::UInt64:
        return readAndScaleData<std::uint64_t>(offset);
    case OmhmDataType::Float32:
        return readAndScaleData<float>(offset);
    case OmhmDataType::Float64:
        return readAndScaleData<double>(offset);
    }
    return bmcl::None;
}

const HmReader* OmhmReader::clone() const
{
    return this;
}
}
