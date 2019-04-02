#include "mcc/hm/SrtmReader.h"
#include "mcc/hm/SrtmFileCache.h"
#include "mcc/geo/LatLon.h"
#include "mcc/geo/MercatorProjection.h"
#include "mcc/map/MapRect.h"
#include "mcc/geo/Constants.h"

#include <bmcl/Math.h>

#include <tclap/CmdLine.h>

#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QDebug>
#include <QResizeEvent>
#include <chrono>

#include <cmath>
#include <cstdlib>
#include <assert.h>

using namespace mcchm;
using namespace mccgeo;
using namespace mccmap;

double calcDistance(const MercatorProjection& proj, double lat1, double lat2)
{
    double slat = std::sin((bmcl::degreesToRadians(lat2 - lat1)) / 2);
    double a = slat * slat;
    double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));
    return proj.majorAxis() * c;
}

static inline double getChannel(uint8_t c1, uint8_t c2, double frac)
{
    return std::fma(c2 - c1, frac, c1);
}

struct Rgb {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

struct DRgb {
    template <typename T>
    DRgb(T r, T g, T b)
        : r(r)
        , g(g)
        , b(b)
    {
    }

    DRgb(Rgb rgb)
        : r(rgb.r)
        , g(rgb.g)
        , b(rgb.b)
    {
    }

    double r;
    double g;
    double b;
};

static DRgb altitudeToColor(int altitude)
{
    constexpr int schemeSize = 14;
    constexpr int minHeight = 0;
    constexpr int maxHeight = 3500;

    static const Rgb scheme[schemeSize] = {
        //Spectral
        { 94,  79, 162}, //    0 -  250
        { 50, 136, 189}, //  250 -  500
        {102, 194, 165}, //  500 -  750
        {171, 221, 164}, //  750 - 1000
        {230, 245, 152}, // 1000 - 1250
        {255, 255, 191}, // 1250 - 1500
        {254, 224, 139}, // 1500 - 1750
        {253, 174,  97}, // 1750 - 2000
        {244, 109,  67}, // 2000 - 2250
        {213,  62,  79}, // 2250 - 2500
        {158,   1,  66}, // 2500 - 2750

        //RdYlGn
        //{  0, 104,  55},
        //{ 26, 152,  80},
        //{102, 189,  99},
        //{166, 217, 106},
        //{217, 239, 139},
        //{255, 255, 191},
        //{254, 224, 139},
        //{253, 174,  97},
        //{244, 109,  67},
        //{215,  48,  39},
        //{165,   0,  38},
        // greys
        {200, 200, 200}, // 2750 - 3000
        {225, 225, 225}, // 3000 - 3250
        {255, 255, 255}, // 3250 - 3500
    };

    if (altitude <= minHeight) {
        return scheme[0];
    } else if (altitude >= maxHeight) {
        return scheme[schemeSize - 1];
    }
    double loc = (schemeSize - 1) * double(altitude) / (maxHeight - minHeight);
    double integral;
    double frac = std::modf(loc, &integral);
    int i = (int)integral;
    return DRgb(getChannel(scheme[i].r, scheme[i+1].r, frac),
                getChannel(scheme[i].g, scheme[i+1].g, frac),
                getChannel(scheme[i].b, scheme[i+1].b, frac));
}

static void cleanQImage(void* info)
{
    uint32_t* ptr = (uint32_t*)info;
    delete [] ptr;
}

// https://openlayers.org/en/latest/examples/shaded-relief.html
// http://pro.arcgis.com/en/pro-app/tool-reference/3d-analyst/how-hillshade-works.htm
template <typename T>
QImage shade(const T* elevationData, const T* xDeltasAtLat, const T* yDeltasAtLat, std::size_t dataWidth, std::size_t dataHeight)
{
    std::size_t imgWidth = dataWidth - 2;
    std::size_t imgHeight = dataHeight - 2;
    uint32_t* imgData = new uint32_t[imgWidth * imgHeight];

    constexpr double pi = bmcl::pi<double>();
    constexpr double twoPi = pi * 2;
    constexpr double halfPi = pi / 2;
    constexpr double sunElDeg = 10;
    constexpr double sunAzDeg = 315;
    constexpr double sunEl = bmcl::degreesToRadians(sunElDeg);
    constexpr double sunAz = bmcl::degreesToRadians(sunAzDeg);
    constexpr double zFactor = 100;

    double cosSunEl = std::cos(sunEl);
    double sinSunEl = std::sin(sunEl);

    for (std::size_t yi = 1; yi < (dataHeight - 1); yi++) {
        double sumDx =  xDeltasAtLat[yi] * 8;
        for (std::size_t xi = 1; xi < (dataWidth - 1); xi++) {
            //      3x3
            //
            //       x
            //   [a, b, c]
            // y [d, e, f]
            //   [g, h, i]

            double sumDy = (yDeltasAtLat[yi] + yDeltasAtLat[yi + 1]) * 4;

            std::size_t offset = (yi - 1) * dataWidth + xi;
            T a = elevationData[offset - 1];
            T b = elevationData[offset];
            T c = elevationData[offset + 1];
            offset += dataWidth;
            T d = elevationData[offset - 1];
            T e = elevationData[offset];
            T f = elevationData[offset + 1];
            offset += dataWidth;
            T g = elevationData[offset - 1];
            T h = elevationData[offset];
            T i = elevationData[offset + 1];

            double dzdx = double((c + 2 * f + i) - (a + 2 * d + g)) / sumDx;
            double dzdy = double((g + 2 * h + i) - (a + 2 * b + c)) / sumDy;

            double slope = std::atan(zFactor * std::sqrt(dzdx * dzdx + dzdy * dzdy));

            double aspect = std::atan2(dzdy, -dzdx);
            if (aspect < 0) {
                aspect = halfPi - aspect;
            } else if (aspect > halfPi) {
                aspect = twoPi - aspect + halfPi;
            } else {
                aspect = halfPi - aspect;
            }

            DRgb color = altitudeToColor(e);

            double cosIncidence = sinSunEl * std::cos(slope) + cosSunEl * std::sin(slope) * std::cos(sunAz - aspect);
            constexpr double colorCoeff = 0.7;
            constexpr double shadeCoeff = 1.0 - colorCoeff;
            double v = std::fma(128.0, cosIncidence, + 127.0) * shadeCoeff;
            imgData[(yi - 1) * imgWidth + xi - 1] =  qRgb(uint8_t(std::fma(color.r, colorCoeff, v)),
                                                          uint8_t(std::fma(color.g, colorCoeff, v)),
                                                          uint8_t(std::fma(color.b, colorCoeff, v)));
        }
    }

    return QImage((uchar*)imgData, imgWidth, imgHeight, QImage::Format::Format_RGB32, cleanQImage, imgData);
}

class TileViewWidget : public QWidget {
public:
    static constexpr std::size_t tileSize = 256;
    static constexpr std::size_t lineSize = tileSize + 2;

    TileViewWidget(const SrtmReader* reader, unsigned z, unsigned x, unsigned y)
    {
        QImage img00 = create(reader, z, x    , y    );
        QImage img01 = create(reader, z, x    , y + 1);
        QImage img10 = create(reader, z, x + 1, y    );
        QImage img11 = create(reader, z, x + 1, y + 1);

        _img = QImage(tileSize * 2, tileSize * 2, QImage::Format_RGB32);

        QPainter p;
        p.begin(&_img);
        p.drawImage(0       ,        0, img00);
        p.drawImage(0       , tileSize, img01);
        p.drawImage(tileSize,        0, img10);
        p.drawImage(tileSize, tileSize, img11);
        p.end();

        for (int x = 0; x < tileSize * 2; x++) {
            //_img.setPixel(x, 256, qRgb(255, 0, 0));
        }

        _img.save("tiletooltest.png");
        _scaledImg = _img;
        resize(tileSize * 2, tileSize * 2);
    }

    QImage create(const SrtmReader* reader, unsigned z, unsigned x, unsigned y)
    {
        assert(z != 0);

        MapRect rect;
        rect.setZoomLevel(z);

        LatLon cornerLatLon = rect.latLon(QPoint(x * 256, y * 256));
        qDebug() << cornerLatLon.latitude() << cornerLatLon.longitude();

        auto time = std::chrono::system_clock::now();

        int xoffset = x * 256 - 1; //FIXME
        int yoffset = y * 256 - 1; //FIXME

        double* data = new double[lineSize * (lineSize + 4)]; // +4 rows

        double* lats         = data;
        double* lons         = data + lineSize;
        double* yDeltasAtLat = data + lineSize * 2;
        double* xDeltasAtLat = data + lineSize * 3;
        double* altData      = data + lineSize * 4;

        for (int yi = 0; yi < lineSize; yi++) {
            double lat = rect.lat(yoffset + yi);
            lats[yi] = lat;
            xDeltasAtLat[yi] = rect.projection().parallelCircumference(lat) / rect.maxMapSize();
            for (int xi = 0; xi < lineSize; xi++) {
                lons[xi] = rect.lon(xoffset + xi);
            }
        }
        for (int yi = 0; yi < (lineSize - 1); yi++) {
            yDeltasAtLat[yi] = calcDistance(rect.projection(), lats[yi], lats[yi + 1]);
        }

        double* altIt = altData;
        for (int yi = 0; yi < lineSize; yi++) {
            double lat = lats[yi];
            for (int xi = 0; xi < lineSize; xi++) {
                *altIt = reader->readAltitude(LatLon(lat, lons[xi])).unwrapOr(0);
                altIt++;
            }
        }

        QImage img = shade(altData, xDeltasAtLat, yDeltasAtLat, lineSize, lineSize);
        delete [] data;
        auto timeDelta = std::chrono::system_clock::now() - time;
        qDebug() << std::chrono::duration_cast<std::chrono::milliseconds>(timeDelta).count();
        return img;
    }

    void resizeEvent(QResizeEvent* event) override
    {
        int size = std::min(event->size().width(), event->size().height());
        _scaledImg = _img.scaled(QSize(size, size), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        update();
    }

    void paintEvent(QPaintEvent* event) override
    {
        QPainter p(this);
        p.fillRect(rect(), Qt::black);
        p.drawImage(0, 0, _scaledImg);
    }

private:
    QImage _img;
    QImage _scaledImg;
};

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    TCLAP::CmdLine cmdLine("mcc");
    TCLAP::ValueArg<std::string> srtmPathArg("p", "srtm-path", "Srtm path", true, "", "path");
    TCLAP::ValueArg<unsigned> zoomArg("z", "zoom", "zoom", true, 0.0, "");
    TCLAP::ValueArg<unsigned> xArg("x", "x", "X", true, 0.0, "");
    TCLAP::ValueArg<unsigned> yArg("y", "y", "Y", true, 0.0, "");

    cmdLine.add(&srtmPathArg);
    cmdLine.add(&zoomArg);
    cmdLine.add(&xArg);
    cmdLine.add(&yArg);
    cmdLine.parse(argc, argv);

    mcchm::Rc<mcchm::RcGeod> geod = new mcchm::RcGeod(mccgeo::wgs84a<double>(), mccgeo::wgs84f<double>());
    mcchm::Rc<SrtmFileCache> cache = new SrtmFileCache(QString::fromStdString(srtmPathArg.getValue()), 10);
    SrtmReader reader(geod.get(), cache.get());

    TileViewWidget w(&reader, zoomArg.getValue(), xArg.getValue(), yArg.getValue());
    w.show();

    return app.exec();
}

