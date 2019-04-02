#pragma once

#include "mcc/hm/Config.h"
#include "mcc/hm/Rc.h"
#include "mcc/hm/Altitude.h"

#include <bmcl/Fwd.h>

class QString;

namespace mcchm {

class MCC_HM_DECLSPEC Gl1File : public RefCountable {
public:
    static bmcl::OptionRc<const Gl1File> load(const QString& dirPath, int latIndex, int lonIndex);
    static Rc<const Gl1File> createInvalid();

    SrtmAltitude readHeight(double latFrac, double lonFrac) const;
    Altitude readSampledHeight(double latFrac, double lonFrac) const;
    bool isValid() const;

private:
    Gl1File() = default;

    uint16_t* _data;
};

inline bool Gl1File::isValid() const
{
    return _data != nullptr;
}

struct Gl1FileDesc {
    Gl1FileDesc();
    Gl1FileDesc(int latIndex, int lonIndex, const Rc<const Gl1File>& file);
    Gl1FileDesc(int latIndex, int lonIndex, Rc<const Gl1File>&& file);

    int latIndex;
    int lonIndex;
    Rc<const Gl1File> file;
};

}
