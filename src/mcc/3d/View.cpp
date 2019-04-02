#include "mcc/3d/View.h"
#include "mcc/3d/Scene.h"

#include <QResizeEvent>

namespace mcc3d {

View::View(QWidget *parent) :
    QGraphicsView(parent),
    _scene(nullptr)
{}

View::View(Scene *scene, QWidget *parent) :
    QGraphicsView(scene, parent),
    _scene(scene)
{}

View::~View(){}

void View::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);

    if(_scene != nullptr)
        _scene->updateSceneSize(event->size());
}
}
