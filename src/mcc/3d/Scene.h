#pragma once

#include "mcc/Config.h"

#include <VasnecovScene>

namespace mcc3d {

class MCC_3D_DECLSPEC Scene : public VasnecovScene
{
    Q_OBJECT

public:
    explicit Scene(VasnecovUniverse *univ, QObject *parent = nullptr);
    ~Scene() override;

public slots:
    virtual void updateSceneSize(const QSizeF &size);

protected:
    Vasnecov::Line mousePositionLine(const QPointF &mousePosition);
    // Point under mouse cursor at the same plane (vertical or horizontal) with basePoint
    QVector3D mousePositionFromPoint(const QPointF &mouseAtScreen,
                                     const QVector3D& basePoint,
                                     const QVector3D& limits = QVector3D(),
                                     bool horizontalPlane = true);

    VasnecovWorld* _world;

    Q_DISABLE_COPY(Scene)
};
}
