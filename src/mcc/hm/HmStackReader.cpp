#include "mcc/hm/HmStackReader.h"

namespace mcchm {

HmStackReader::HmStackReader(const RcGeod* wgs84Geod)
    : HmReader(wgs84Geod)
{
}

HmStackReader::~HmStackReader()
{
}

Altitude HmStackReader::readAltitude(mccgeo::LatLon latLon, double prec) const
{
    for (const auto& hm : _readers) {
        Altitude alt = hm->readAltitude(latLon, prec);
        if (alt.isSome()) {
            return alt;
        }
    }
    return bmcl::None;
}

const HmStackReader* HmStackReader::clone() const
{
    HmStackReader* reader = new HmStackReader(geod());
    reader->_readers.reserve(_readers.size());
    for (const auto& hm : _readers) {
        reader->_readers.emplace_back(hm->clone());
    }
    return reader;
}

std::size_t HmStackReader::size() const
{
    return _readers.size();
}

void HmStackReader::appendReader(const HmReader* reader)
{
    assert(reader != 0);
    _readers.emplace_back(reader);
}

bool HmStackReader::insertReader(std::size_t pos, const HmReader* reader)
{
    if (pos > _readers.size()) {
        return false;
    }

    _readers.emplace(_readers.begin() + pos, reader);
    return true;
}

bool HmStackReader::removeReader(std::size_t pos)
{
    if (pos >= _readers.size()) {
        return false;
    }
    _readers.erase(_readers.begin() + pos);
    return true;
}

void HmStackReader::clear()
{
    _readers.clear();
}

}
