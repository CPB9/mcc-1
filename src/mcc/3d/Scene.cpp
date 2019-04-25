#include "mcc/3d/Scene.h"

#include <bmcl/Logging.h>
#include <VasnecovUniverse>

namespace mcc3d {

Scene::Scene(VasnecovUniverse *univ, QObject *parent) :
    VasnecovScene(parent),
    _world(nullptr)
{
    VasnecovScene::setUniverse(univ);

    if(!universe())
        BMCL_CRITICAL() << "Wrong Universe!";
}

Scene::~Scene() {}

void Scene::updateSceneSize(const QSizeF &size)
{
    setSceneRect(0.0, 0.0, size.width(), size.height());
}

Vasnecov::Line Scene::mousePositionLine(const QPointF &mousePosition)
{
    if(_world == nullptr)
        return Vasnecov::Line();

    return _world->unprojectPointToLine(static_cast<float>(mousePosition.x()),
                                        static_cast<float>(windowHeight() - mousePosition.y()));
}

QVector3D Scene::mousePositionFromPoint(const QPointF& mouseAtScreen, const QVector3D& basePoint, const QVector3D& limits, bool horizontalPlane)
{
    Vasnecov::Line line = mousePositionLine(mouseAtScreen);
    if(line.isNull())
        return basePoint;

    QVector3D planeNorm(0.0f, 0.0f, 1.0f); // Only horizontal moving

    if(!horizontalPlane)
    {
        planeNorm = QVector3D(_world->camera().position() - basePoint); // Only vertical moving
        planeNorm.normalize();
    }

    QVector3D diffVector = QVector3D(line.p2() - line.p1());
    float denominator = QVector3D::dotProduct(planeNorm, diffVector);

    // Line is not parallel to plane
    if(qFuzzyIsNull(denominator))
        return basePoint;

    // basePoint is a point at the plane
    float numerator = QVector3D::dotProduct(planeNorm, basePoint - line.p1());
    float u = numerator / denominator;
    QVector3D intersection = line.p1() + u * diffVector;

    if(!limits.isNull())
    {
        if(std::abs(intersection.x()) > limits.x())
            intersection.setX(intersection.x() >= 0.0f ? limits.x() : limits.x() * -1.0f);

        if(std::abs(intersection.y()) > limits.y())
            intersection.setY(intersection.y() >= 0.0f ? limits.y() : limits.y() * -1.0f);

        if(std::abs(intersection.z()) > limits.z())
            intersection.setZ(intersection.z() >= 0.0f ? limits.z() : limits.z() * -1.0f);
    }

    if(horizontalPlane)
        return QVector3D(intersection.x(), intersection.y(), basePoint.z());
    else
        return QVector3D(basePoint.x(), basePoint.y(), intersection.z());
}

}
