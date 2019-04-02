#pragma once

#include "mcc/Config.h"

#include <VasnecovProduct>

class VasnecovUniverse;
class VasnecovWorld;

class VasnecovFigure;
class VasnecovLabel;

namespace mcc3d {

class MCC_3D_DECLSPEC EmptyModel
{
public:
    EmptyModel(VasnecovUniverse *universe,
               VasnecovWorld *world);

    virtual ~EmptyModel(){}

    static float angleToRange360(float deg);
    static float angleToRange180(float deg);
    static float angleToRange90(float deg);

    void setTools(VasnecovUniverse *universe, VasnecovWorld *world);

    void setUniverse(VasnecovUniverse *universe);
    VasnecovUniverse *universe() const {return _universe;}
    void setWorld(VasnecovWorld *world);
    VasnecovWorld *world() const {return _world;}

    virtual void setVisible(bool visible = true);
    bool isVisible() const {return _visible;}
    void hide() {setVisible(false);}
    void show() {setVisible(true);}

    void setPosition(const QVector3D &coord, const QVector3D &angles);
    virtual void setCoordinates(const QVector3D &c);
    QVector3D coordinates() const {return _coordinates;}
    virtual void setAngles(const QVector3D &a);
    QVector3D angles() const {return _angles;}

    void removeProduct(VasnecovProduct *&product);
    void removeLabel(VasnecovLabel *&label);
    void removeFigure(VasnecovFigure *&figure);

    virtual void updateDrawing();

protected:
    bool hasTools() const {return _universe && _world;}

private:
    VasnecovUniverse*   _universe;
    VasnecovWorld*      _world;
    QVector3D           _coordinates;
    QVector3D           _angles;
    bool                _visible;

private:
    Q_DISABLE_COPY(EmptyModel)
};

class MCC_3D_DECLSPEC ProductModel : public EmptyModel
{
public:
    ProductModel(VasnecovUniverse *u,
                 VasnecovWorld *w,
                 const QString &n = QString());
    ~ProductModel() override;

    void setVisible(bool visible = true) override;

    void setCoordinates(const QVector3D &c) override;
    void setAngles(const QVector3D &a) override;

    void setParent(VasnecovProduct *parent);

    VasnecovProduct *addPart(const QString &name,
                             const QString &mesh,
                             const QString &texture,
                             VasnecovProduct *parent = nullptr);
    VasnecovProduct *addPart(const QString &name,
                             const QString &mesh,
                             VasnecovProduct *parent = nullptr);
    VasnecovProduct *addAssembly(const QString &name,
                                 VasnecovProduct *parent = nullptr);

    VasnecovLabel *addLabel(const QString &name,
                            GLfloat width,
                            GLfloat height,
                            const QString &texture,
                            const VasnecovAbstractElement *element = nullptr);
    VasnecovLabel *addLabel(const QString &name,
                            GLfloat width,
                            GLfloat height,
                            const VasnecovAbstractElement *element = nullptr);
    VasnecovFigure *addFigure(const QString &name,
                              const VasnecovAbstractElement *element = nullptr);

protected:
    bool hasWorkModel() const {return hasTools() && _model;}

    VasnecovProduct *model() const {return _model;}
    VasnecovProduct *modelParent() const;

private:
    VasnecovProduct *_model;

private:
    Q_DISABLE_COPY(ProductModel)
};

inline void EmptyModel::setTools(VasnecovUniverse *universe, VasnecovWorld *world)
{
    _universe = universe;
    _world = world;
}

inline void EmptyModel::setUniverse(VasnecovUniverse *universe)
{
    _universe = universe;
}

inline void EmptyModel::setWorld(VasnecovWorld *world)
{
    _world = world;
}

inline VasnecovProduct *ProductModel::modelParent() const
{
    if(_model != nullptr)
        return _model->parent();

    return nullptr;
}
}
