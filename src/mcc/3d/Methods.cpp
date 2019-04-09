#include "Methods.h"
#include "mcc/geo/Vector3D.h"

#include <bmcl/Math.h>

#include <QVector2D>
#include <QVector3D>
#include <QVector4D>

#include <VasnecovWorld>

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

float angleToRange360(float deg)
{
    if(deg > 0.0f)
        deg = std::fmod(deg, 360.0f);
    if(deg < 0.0f)
        deg = std::fmod(deg, 360.0f) + 360.0f;
    return deg;
}

float angleToRange180(float deg)
{
    deg = angleToRange360(deg);
    if((360 - deg) < 180) // (-180; 180]
        deg = deg - 360;

    return deg;
}

float angleToRange90(float deg)
{
    deg = angleToRange180(deg);
    if(deg < 0.0f)
        deg = 0.0f;
    if(deg > 90.0f)
        deg = 90.0f;
    return deg;
}

float viewPlaneHorizontalAngle(VasnecovWorld* world)
{
    if(world == nullptr)
        return 0.0f;

    QVector3D viewVector = world->cameraPosition() - world->cameraTarget();
    viewVector.setZ(0.0f);
    viewVector.normalize();
    QVector3D yVector(0.0f, -1.0f, 0.0f);

    float angle = std::atan2(QVector3D::dotProduct(QVector3D(0.0f, 0.0f, 1.0f), QVector3D::crossProduct(viewVector, yVector)),
                             QVector3D::dotProduct(viewVector, yVector));
    return - mcc3d::angleToRange360(bmcl::radiansToDegrees(angle));
}


}
