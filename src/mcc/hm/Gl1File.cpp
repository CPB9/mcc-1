#include "mcc/hm/Gl1File.h"

#include <bmcl/Endian.h>
#include <bmcl/Assert.h>
#include <bmcl/OptionRc.h>

#include <QDebug>
#include <QString>
#include <QFile>

//#include <minizip/unzip.h>
#include <lz4frame.h>

#include <limits>
#include <cmath>
#include <cassert>

namespace mcchm {

constexpr const unsigned srtmgl1MatrixWidth = 3601;
constexpr const unsigned srtmgl1MatrixHeight = 3601;
constexpr const unsigned srtmgl1FileSize = srtmgl1MatrixWidth * srtmgl1MatrixHeight * 2;

bmcl::OptionRc<const Gl1File> Gl1File::load(const QString& dirPath, int latIndex, int lonIndex)
{
    char latChar = 'N';
    char lonChar = 'E';
    if (latIndex < 0) {
        latChar = 'S';
    }
    if (lonIndex < 0) {
        lonChar = 'W';
    }
    QString path = QString(dirPath + "/%1%2%3%4.SRTMGL1.hgt.lz4")
                       .arg(latChar)
                       .arg((unsigned)std::abs(latIndex), 2, 10, QChar('0'))
                       .arg(lonChar)
                       .arg((unsigned)std::abs(lonIndex), 3, 10, QChar('0'));

    QFile qfile(path);
    if (!qfile.open(QFile::ReadOnly)) {
        //qDebug() << "error opening file " << path;
        return bmcl::None;
    }

    void* data = qfile.map(0, qfile.size());
    if (!data) {
        qCritical() << "error memory mapping file " << path;
        return bmcl::None;
    }
    LZ4F_dctx* dctx;
    LZ4F_errorCode_t err = LZ4F_createDecompressionContext(&dctx, LZ4F_VERSION);
    if (LZ4F_isError(err)) {
        qCritical() << "failed to create decompression context " << path << LZ4F_getErrorName(err);
        return bmcl::None;
    }

    size_t srcSize = qfile.size();
    const uint8_t* srcIt = (const uint8_t*)data;
    const uint8_t* srcEnd = srcIt + srcSize;

    struct Container {
        Gl1File file;
        uint16_t data[srtmgl1MatrixWidth * srtmgl1MatrixHeight];
    };

    auto cont = new Container;
    Rc<Gl1File> file = &cont->file;
    assert(uintptr_t(cont) == uintptr_t(file.get()));
    file->_data = &cont->data[0];

    uint8_t* destIt = (uint8_t*)file->_data;
    size_t dstSize = srtmgl1FileSize;
    uint8_t* destEnd = destIt + dstSize;

    while (true) {
        size_t rv = LZ4F_decompress(dctx, destIt, &dstSize, srcIt, &srcSize, NULL);
        if (rv == 0) {
            break;
        }
        if (LZ4F_isError(rv)) {
            qWarning() << "failed to create decompress chunk " << path << LZ4F_getErrorName(rv);
            LZ4F_freeDecompressionContext(dctx);
            return bmcl::None;
        }
        srcIt += srcSize;
        destIt += dstSize;
        if (srcIt >= srcEnd || destIt >= destEnd) {
            qWarning() << "internal decompression error " << path;
            LZ4F_freeDecompressionContext(dctx);
            return bmcl::None;
        }
        dstSize = destEnd - destIt;
        srcSize = srcEnd - srcIt;
    }
    LZ4F_freeDecompressionContext(dctx);

    return Rc<const Gl1File>(std::move(file));
}

Rc<const Gl1File> Gl1File::createInvalid()
{
    auto file = new Gl1File;
    file->_data = nullptr;
    return file;
}

SrtmAltitude Gl1File::readHeight(double latFrac, double lonFrac) const
{
    if (!_data) {
        return bmcl::None;
    }
    int row = std::trunc(latFrac * 3600);
    int col = std::trunc(lonFrac * 3600);

    return be16toh(_data[row * srtmgl1MatrixWidth + col]);
}

Altitude Gl1File::readSampledHeight(double latFrac, double lonFrac) const
{
    assert(latFrac >= 0);
    assert(lonFrac >= 0);

    assert(latFrac <= 1);
    assert(lonFrac <= 1);

    if (!_data) {
        return bmcl::None;
    }

    double rowIntegral;
    double rowCoeff = 1.0 - std::modf(latFrac * 3600, &rowIntegral);

    double colIntegral;
    double colCoeff = 1.0 - std::modf(lonFrac * 3600, &colIntegral);

    unsigned row = rowIntegral;
    unsigned col = colIntegral;
    //[a, b]
    //[c, d]

    std::size_t offset = row * srtmgl1MatrixWidth + col;
    double a = (int16_t)be16toh(_data[offset]);
    double b = (int16_t)be16toh(_data[offset + 1]);
    double avg1 = std::fma(colCoeff, (a - b), b);

    offset += srtmgl1MatrixWidth;
    double c = (int16_t)be16toh(_data[offset]);
    double d = (int16_t)be16toh(_data[offset + 1]);
    double avg2 = std::fma(colCoeff, (c - d), d);

    return std::fma(rowCoeff, (avg1 - avg2), avg2);
}

Gl1FileDesc::Gl1FileDesc()
    : latIndex(std::numeric_limits<int>::min())
    , lonIndex(std::numeric_limits<int>::min())
    , file(nullptr)
{
}

Gl1FileDesc::Gl1FileDesc(int latIndex, int lonIndex, const Rc<const Gl1File>& file)
    : latIndex(latIndex)
    , lonIndex(lonIndex)
    , file(file)
{
}

Gl1FileDesc::Gl1FileDesc(int latIndex, int lonIndex, Rc<const Gl1File>&& file)
    : latIndex(latIndex)
    , lonIndex(lonIndex)
    , file(std::move(file))
{
}
}
