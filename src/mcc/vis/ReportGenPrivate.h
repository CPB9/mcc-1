#pragma once

#include "mcc/Config.h"
#include "mcc/vis/Rc.h"

#include <QThread>

#include <thread>

namespace mccvis {

class Region;

class ReportGenPrivate : public QThread {
    Q_OBJECT
public:
    ReportGenPrivate();
    ~ReportGenPrivate();

    QString path;
    Rc<const Region> region;

protected:
    void run() override;

signals:
    void finished(int rv);
    void progressChanged(int value);
};
}
