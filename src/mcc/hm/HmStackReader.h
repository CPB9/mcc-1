#pragma once

#include "mcc/hm/Config.h"
#include "mcc/hm/Rc.h"
#include "mcc/hm/HmReader.h"

namespace mcchm {

class MCC_HM_DECLSPEC HmStackReader : public HmReader  {
public:
    HmStackReader(const RcGeod* wgs84Geod);
    ~HmStackReader() override;

    Altitude readAltitude(mccgeo::LatLon latLon, double prec) const override;
    const HmStackReader* clone() const override;

    std::size_t size() const;
    void appendReader(const HmReader* reader);
    bool insertReader(std::size_t pos, const HmReader* reader);
    bool removeReader(std::size_t pos);
    void clear();

protected:
    std::vector<Rc<const HmReader>> _readers;
};

}

