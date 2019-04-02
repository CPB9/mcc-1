#pragma once

#include "mcc/hm/Config.h"
#include "mcc/hm/Rc.h"
#include "mcc/hm/HmReader.h"
#include "mcc/geo/Fwd.h"

#include <bmcl/Fwd.h>

#include <QFile>

#include <cstdint>

class QString;

namespace mcchm {

enum class OmhmDataType : std::uint32_t {
    Int8 = 0,
    Int16 = 1,
    Int32 = 2,
    Int64 = 3,
    UInt8 = 4,
    UInt16 = 5,
    UInt32 = 6,
    UInt64 = 7,
    Float32 = 8,
    Float64 = 9,
};

class MCC_HM_DECLSPEC OmhmReader : public HmReader  {
public:
    ~OmhmReader() override;

    static constexpr std::uint32_t magicHeader = 0xac51f4f4;

    static std::uint32_t crc32(const void* data, std::size_t size);

    static bmcl::Result<Rc<OmhmReader>, QString> create(const RcGeod* wgs84Geod, const QString& filePath);

    Altitude readAltitude(mccgeo::LatLon latLon, double precisionArcSecond) const override;
    const HmReader* clone() const override;

private:
    bmcl::Option<QString> open(const QString& filePath);

    OmhmReader(const RcGeod* wgs84Geod);

    template <typename T>
    Altitude readAndScaleData(std::size_t offset) const;

    OmhmDataType _dtype;
    std::uint32_t _cellSize;
    std::uint32_t _width;
    std::uint32_t _height;
    double _t0;
    double _t1;
    double _t2;
    double _t3;
    double _t4;
    double _t5;
    double _t2t4_t1t5;
    double _noDataValue;
    double _scale;
    double _offset;

    const uint8_t* _data;
    QFile _file;
    Rc<mccgeo::CoordinateConverter> _conv;
};
}

