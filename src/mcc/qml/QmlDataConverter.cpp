#include "mcc/qml/QmlDataConverter.h"

#include <QObject>
#include <QMetaObject>
#include <QList>

#include <bmcl/MemWriter.h>
#include <bmcl/MemReader.h>

namespace mccqml {

QmlDataConverter::QmlDataConverter(QObject* parent)
    : QObject(parent)
{
}

QmlDataConverter::~QmlDataConverter()
{
}

float QmlDataConverter::toFloatLe(const QList<int>& data)
{
    if ((size_t)data.size() < sizeof(float))
        return 0.0;

    auto vec = toVector(data);
    bmcl::MemReader reader(vec.data(), vec.size());
    return reader.readFloat32Le();
}

float QmlDataConverter::toFloatBe(const QList<int>& data)
{
    if ((size_t)data.size() < sizeof(float))
        return 0.0;

    auto vec = toVector(data);
    bmcl::MemReader reader(vec.data(), vec.size());
    return reader.readFloat32Be();
}

double QmlDataConverter::toDoubleLe(const QList<int>& data)
{
    if ((size_t)data.size() < sizeof(double))
        return 0.0;

    auto vec = toVector(data);
    bmcl::MemReader reader(vec.data(), vec.size());
    return reader.readFloat64Le();
}

double QmlDataConverter::toDoubleBe(const QList<int>& data)
{
    if ((size_t)data.size() < sizeof(double))
        return 0.0;

    auto vec = toVector(data);
    bmcl::MemReader reader(vec.data(), vec.size());
    return reader.readFloat64Be();
}

int QmlDataConverter::toInt32Le(const QList<int>& data)
{
    if ((size_t)data.size() < sizeof(int32_t))
        return 0;

    auto vec = toVector(data);
    bmcl::MemReader reader(vec.data(), vec.size());
    return reader.readInt32Le();
}

int QmlDataConverter::toInt32Be(const QList<int>& data)
{
    if ((size_t)data.size() < sizeof(int32_t))
        return 0;

    auto vec = toVector(data);
    bmcl::MemReader reader(vec.data(), vec.size());
    return reader.readInt32Be();
}

unsigned int QmlDataConverter::toUInt32Le(const QList<int>& data)
{
    if ((size_t)data.size() < sizeof(uint32_t))
        return 0;

    auto vec = toVector(data);
    bmcl::MemReader reader(vec.data(), vec.size());
    return reader.readUint32Le();
}

unsigned int QmlDataConverter::toUInt32Be(const QList<int>& data)
{
    if ((size_t)data.size() < sizeof(uint32_t))
        return 0;

    auto vec = toVector(data);
    bmcl::MemReader reader(vec.data(), vec.size());
    return reader.readUint32Be();
}

unsigned int QmlDataConverter::toUInt16Le(const QList<int>& data)
{
    if ((size_t)data.size() < sizeof(uint16_t))
        return 0;
    auto vec = toVector(data);
    bmcl::MemReader reader(vec.data(), vec.size());
    return reader.readUint16Le();
}

unsigned int QmlDataConverter::toUInt16Be(const QList<int>& data)
{
    if ((size_t)data.size() < sizeof(uint16_t))
        return 0;

    auto vec = toVector(data);
    bmcl::MemReader reader(vec.data(), vec.size());
    return reader.readUint16Be();
}

int QmlDataConverter::toInt16Le(const QList<int>& data)
{
    if ((size_t)data.size() < sizeof(int16_t))
        return 0;

    auto vec = toVector(data);
    bmcl::MemReader reader(vec.data(), vec.size());
    return reader.readInt16Le();
}

int QmlDataConverter::toInt16Be(const QList<int>& data)
{
    if ((size_t)data.size() < sizeof(int16_t))
        return 0;

    auto vec = toVector(data);
    bmcl::MemReader reader(vec.data(), vec.size());
    return reader.readInt16Be();
}

int QmlDataConverter::toInt8(const QList<int>& data)
{
    if ((size_t)data.size() < sizeof(int8_t))
        return 0;

    auto vec = toVector(data);
    bmcl::MemReader reader(vec.data(), vec.size());
    return reader.readInt8();
}

int QmlDataConverter::toUInt8(const QList<int>& data)
{
    if ((size_t)data.size() < sizeof(uint8_t))
        return 0;

    auto vec = toVector(data);
    bmcl::MemReader reader(vec.data(), vec.size());
    return reader.readUint8();
}

std::vector<uint8_t> QmlDataConverter::toVector(const QList<int>& data) const
{
    std::vector<uint8_t> vec;
    for (auto x : data)
        vec.push_back((uint8_t)x);
    return vec;
}
}
