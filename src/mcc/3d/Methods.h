#pragma once

#include "mcc/Config.h"

class QVector2D;
class QVector3D;
class QVector4D;

namespace mccgeo {
class Vector3D;
}

namespace mcc3d {

MCC_3D_DECLSPEC QVector2D toQVector2D(const mccgeo::Vector3D& vector);
MCC_3D_DECLSPEC QVector3D toQVector3D(const mccgeo::Vector3D& vector);
MCC_3D_DECLSPEC QVector4D toQVector4D(const mccgeo::Vector3D& vector);

MCC_3D_DECLSPEC mccgeo::Vector3D toVector3D(const QVector3D& vector);
}
