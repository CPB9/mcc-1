#pragma once

#include "mcc/Config.h"
#include <QObject>

template <typename T>
class QList;

namespace mccqml {

class MCC_QML_DECLSPEC QmlDataConverter : public QObject
{
    Q_OBJECT

public:
    QmlDataConverter(QObject* parent = 0);
    ~QmlDataConverter();

    Q_INVOKABLE float toFloatLe(const QList<int>& data);
    Q_INVOKABLE float toFloatBe(const QList<int>& data);
    Q_INVOKABLE double toDoubleLe(const QList<int>& data);
    Q_INVOKABLE double toDoubleBe(const QList<int>& data);
    Q_INVOKABLE int toInt32Le(const QList<int>& data);
    Q_INVOKABLE int toInt32Be(const QList<int>& data);
    Q_INVOKABLE unsigned int toUInt32Le(const QList<int>& data);
    Q_INVOKABLE unsigned int toUInt32Be(const QList<int>& data);
    Q_INVOKABLE unsigned int toUInt16Le(const QList<int>& data);
    Q_INVOKABLE unsigned int toUInt16Be(const QList<int>& data);
    Q_INVOKABLE int toInt16Le(const QList<int>& data);
    Q_INVOKABLE int toInt16Be(const QList<int>& data);
    Q_INVOKABLE int toInt8(const QList<int>& data);
    Q_INVOKABLE int toUInt8(const QList<int>& data);

private:
    std::vector<uint8_t> toVector(const QList<int>& data) const;
};
}
