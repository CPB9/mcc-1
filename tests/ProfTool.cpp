#include "mcc/hm/SrtmReader.h"
#include "mcc/vis/Profile.h"
#include "mcc/vis/ProfileViewer.h"
#include "mcc/vis/RadarParams.h"
#include "mcc/geo/LatLon.h"
#include "mcc/geo/Geod.h"
#include "mcc/geo/Constants.h"

#include <tclap/CmdLine.h>
#include <bmcl/OptionPtr.h>

#include <QApplication>

using namespace mccvis;
using namespace mcchm;
using namespace mccgeo;

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    TCLAP::CmdLine cmdLine("mcc");
    TCLAP::ValueArg<std::string> srtmPathArg("", "srtm-path", "Srtm path", true, "", "path");
    TCLAP::ValueArg<double> latArg("", "lat", "Latitude", true, 0.0, "degrees");
    TCLAP::ValueArg<double> lonArg("", "lon", "Longitude", true, 0.0, "degrees");
    TCLAP::ValueArg<double> dirArg("", "dir", "Direction", true, 0.0, "degrees");
    TCLAP::ValueArg<double> lenArg("", "len", "length", true, 0.0, "meters");
    TCLAP::ValueArg<double> fresnesFreqArg("", "fresnel-freq", "frequency", false, 2800, "MHz");

    cmdLine.add(&srtmPathArg);
    cmdLine.add(&latArg);
    cmdLine.add(&lonArg);
    cmdLine.add(&dirArg);
    cmdLine.add(&lenArg);
    cmdLine.add(&fresnesFreqArg);
    cmdLine.parse(argc, argv);

    ViewParams params;
    params.useFresnelRegion = fresnesFreqArg.isSet();
    params.frequency = fresnesFreqArg.getValue();

    mcchm::Rc<mcchm::RcGeod> geod = new mcchm::RcGeod(mccgeo::wgs84a<double>(), mccgeo::wgs84f<double>());
    mcchm::SrtmReader reader(geod.get(), QString::fromStdString(srtmPathArg.getValue()));
    LatLon start(latArg.getValue(), lonArg.getValue());
    LatLon end;

    geod->direct(start.latitude(), start.longitude(),
                 dirArg.getValue(), lenArg.getValue(),
                 &end.latitude(), &end.longitude(), 0);

    PointVector slice = reader.relativePointProfileAutostep(start, end);
    mccvis::Rc<Profile> prof = new Profile(0, slice, params);

    ProfileViewer viewer;
    viewer.setProfile(prof.get());
    viewer.showMaximized();

    return app.exec();
}
