#include "Methods.h"
#include "mcc/geo/Vector3D.h"
#include <QVector2D>
#include <QVector3D>
#include <QVector4D>

namespace mcc3d {

QVector2D toQVector2D(const mccgeo::Vector3D& vector)
{
    return QVector2D(vector.x(), vector.y());
}

QVector3D toQVector3D(const mccgeo::Vector3D& vector)
{
    return QVector3D(vector.x(), vector.y(), vector.z());
}

QVector4D toQVector4D(const mccgeo::Vector3D& vector)
{
    return QVector4D(vector.x(), vector.y(), vector.z(), 0.0f);
}

mccgeo::Vector3D toVector3D(const QVector3D& vector)
{
    return mccgeo::Vector3D(vector.x(), vector.y(), vector.z());
}


}
