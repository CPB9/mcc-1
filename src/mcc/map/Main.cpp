#include "mcc/map/OmcfCacheWidget.h"

#include <QApplication>
#include <QTreeView>

using namespace mccmap;

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    OmcfCacheWidget view;
    view.show();
    return app.exec();
}
