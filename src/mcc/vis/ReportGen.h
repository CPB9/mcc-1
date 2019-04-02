#pragma once

#include "mcc/vis/Config.h"
#include "mcc/vis/Rc.h"
#include "mcc/vis/ReportConfig.h"

#include <QThread>

class QString;
class QWidget;

namespace mccvis {

class Region;

class MCC_VIS_DECLSPEC ReportGen : public QThread {
    Q_OBJECT
public:
    ReportGen();
    ~ReportGen();

    void generateReport(const Region* region, const QString& path, const ReportConfig& conf);

protected:
    void run() override;

signals:
    void finished(int rv);
    void progressChanged(int value);

private:
    void gen();

    QString _path;
    Rc<const Region> _region;
    ReportConfig _config;

};
}
