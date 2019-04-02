#pragma once

#include <QObject>

namespace mccqml {

class QmlController : public QObject
{
    Q_OBJECT

public:
    ~QmlController();

    Q_INVOKABLE QStringList getActionsList() const;
};

}

