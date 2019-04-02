#pragma once

#include "mcc/hm/Config.h"
#include "mcc/hm/Rc.h"
#include "mcc/hm/Gl1File.h"

#include <bmcl/OptionRc.h>

#include <QString>

#include <deque>
#include <mutex>

namespace mcchm {

class Gl1File;
class Gl30AllFile;
struct Gl1FileDesc;

class MCC_HM_DECLSPEC SrtmFileCache : public RefCountable {
public:
    SrtmFileCache(const QString& rootPath, std::size_t size = 1);
    ~SrtmFileCache();

    const Gl1FileDesc& getInvalidFile();
    const bmcl::OptionRc<const Gl30AllFile>& gl30AllFile() const;

    Gl1FileDesc loadFile(int latIndex, int lonIndex) const;
    void resize(std::size_t size);
    void setPath(const QString& path);

private:
    mutable std::mutex _mutex;
    mutable std::deque<Gl1FileDesc> _files;
    QString _path;
    std::size_t _maxSize;
    Rc<const Gl1File> _invalidFile;
    bmcl::OptionRc<const Gl30AllFile> _gl30AllFile;
};
}
