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

}
