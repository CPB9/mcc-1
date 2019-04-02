#pragma once

#include "mcc/Config.h"

#include <QGraphicsView>

class QResizeEvent;

namespace mcc3d {

class Scene;

class MCC_3D_DECLSPEC View : public QGraphicsView
{
    Q_OBJECT

public:
    explicit View(QWidget *parent = nullptr);
    explicit View(Scene *scene, QWidget *parent = nullptr);
    ~View() override;

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    Scene* _scene;

    Q_DISABLE_COPY(View)
};
}
