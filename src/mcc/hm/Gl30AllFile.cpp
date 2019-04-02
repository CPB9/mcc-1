#include "mcc/hm/Gl30AllFile.h"

#include <bmcl/OptionRc.h>
#include <bmcl/Endian.h>

#include <QDebug>

#include <cmath>

namespace mcchm {

constexpr const unsigned rows = 6000 * 3;
constexpr const unsigned cols = 4800 * 9;
constexpr const unsigned rowSize = cols + 1;
constexpr const unsigned totalFileSize = (cols + 1) * (rows + 1) * 2;

Gl30AllFile::~Gl30AllFile()
{
}

bmcl::OptionRc<const Gl30AllFile> Gl30AllFile::load(const QString& dirPath)
{
    Rc<Gl30AllFile> rv = new Gl30AllFile;
    rv->_data = nullptr;
    QString path = dirPath + "/SRTMGL30_ALL.dem";
    QFile& file = rv->_file;
    file.setFileName(path);
    if (!file.open(QFile::ReadOnly)) {
        qDebug() << "failed to open file" << path;
        return bmcl::None;
    }
    if (file.size() != totalFileSize) {
        qWarning() << "invalid file size" << path;
        return bmcl::None;
    }
    //aligned x86 linux and windows
    rv->_data = (uint16_t*)file.map(0, file.size());
    if (!rv->_data) {
        qCritical() << "failed to map file" << path;
        return bmcl::None;
    }
    return Rc<const Gl30AllFile>(rv);
}

SrtmAltitude Gl30AllFile::readHeight(double lat, double lon) const
{
    if (lat < -60 || lat > 90 || lon < -180 || lon > 180) {
        return bmcl::None;
    }

    int col = std::trunc(double(cols) * (lon + 180.0) / 360);
    int row = std::trunc(double(rows) * (90.0 - lat) / 150);

    return be16toh(_data[row * rowSize + col]);
}

Altitude Gl30AllFile::readSampledHeight(double lat, double lon) const
{
    if (lat < -60 || lat > 90 || lon < -180 || lon > 180) {
        return bmcl::None;
    }

    double rowIntegral;
    double rowCoeff = 1.0 - std::modf(double(rows) * (90.0 - lat) / 150, &rowIntegral);

    double colIntegral;
    double colCoeff = 1.0 - std::modf(double(cols) * (lon + 180.0) / 360, &colIntegral);

    unsigned row = rowIntegral;
    unsigned col = colIntegral;
    //[a, b]
    //[c, d]

    std::size_t offset = row * rowSize + col;
    double a = (int16_t)be16toh(_data[offset]);
    double b = (int16_t)be16toh(_data[offset + 1]);
    double avg1 = std::fma(colCoeff, (a - b), b);

    offset += rowSize;
    double c = (int16_t)be16toh(_data[offset]);
    double d = (int16_t)be16toh(_data[offset + 1]);
    double avg2 = std::fma(colCoeff, (c - d), d);

    return std::fma(rowCoeff, (avg1 - avg2), avg2);
}
}
