#include "mcc/3d/ProductModel.h"
#include <VasnecovUniverse>
#include <VasnecovFigure>
#include <VasnecovLabel>
#include <bmcl/Logging.h>

namespace mcc3d {

EmptyModel::EmptyModel(VasnecovUniverse *universe, VasnecovWorld *world) :
    _universe(universe),
    _world(world),
    _coordinates(), _angles(),
    _visible(true)
{
    if(!_universe)
        BMCL_WARNING() << "Empty Universe!";

    if(!_world)
        BMCL_WARNING() << "Empty World!";
}

void EmptyModel::setVisible(bool visible)
{
    if(_visible != visible)
        _visible = visible;
}

void EmptyModel::setPosition(const QVector3D &coord, const QVector3D &angles)
{
    setCoordinates(coord);
    setAngles(angles);
}

void EmptyModel::setCoordinates(const QVector3D &c)
{
    if(_coordinates != c)
        _coordinates = c;
}

void EmptyModel::setAngles(const QVector3D &a)
{
    if(_angles != a)
        _angles = a;
}

void EmptyModel::removeProduct(VasnecovProduct *&product)
{
    if(universe())
    {
        if(universe()->removeProduct(product))
            product = nullptr;
    }
}

void EmptyModel::removeLabel(VasnecovLabel *&label)
{
    if(universe())
    {
        if(universe()->removeLabel(label))
            label = nullptr;
    }
}

void EmptyModel::removeFigure(VasnecovFigure *&figure)
{
    if(universe())
    {
        if(universe()->removeFigure(figure))
            figure = nullptr;
    }
}

void EmptyModel::updateDrawing()
{}

ProductModel::ProductModel(VasnecovUniverse *u, VasnecovWorld *w, const QString &n) :
    EmptyModel(u, w),
    _model(nullptr)
{
    if(hasTools())
    {
        _model = universe()->addAssembly(n, world());
        if(_model == nullptr)
            BMCL_WARNING() << n << ": can't create node 3D-model";
    }
    else
    {
        BMCL_WARNING() << n << ": need to set tools for creating";
    }
}

ProductModel::~ProductModel()
{
    removeProduct(_model);
}

void ProductModel::setVisible(bool visible)
{
    if(_model && visible != isVisible())
    {
        EmptyModel::setVisible(visible);

        _model->setVisible(visible);
    }
}

void ProductModel::setCoordinates(const QVector3D &c)
{
    if(_model && coordinates() != c)
    {
        EmptyModel::setCoordinates(c);

        _model->setCoordinates(c);
    }
}

void ProductModel::setAngles(const QVector3D &a)
{
    if(_model && angles() != a)
    {
        EmptyModel::setAngles(a);

        _model->setAngles(a);
    }
}

void ProductModel::setParent(VasnecovProduct *parent)
{
    if(model())
    {
        model()->changeParent(parent);
    }
}

VasnecovProduct *ProductModel::addPart(const QString &name, const QString &mesh, const QString &texture, VasnecovProduct *parent)
{
    if(hasWorkModel())
    {
        VasnecovProduct *product(nullptr);
        QString fullName = model()->name() + " " + name;
        if(!parent)
        {
            parent = model();
        }

        if(!texture.isEmpty())
        {
            product = universe()->addPart(fullName, world(), mesh, texture, parent);
        }
        else
        {
            product = universe()->addPart(fullName, world(), mesh, parent);
        }

        if(product)
        {
            return product;
        }
    }

    BMCL_WARNING() << "Can't create child part of 3D-model: " << name;
    return nullptr;
}

VasnecovProduct *ProductModel::addPart(const QString &name, const QString &mesh, VasnecovProduct *parent)
{
    return addPart(name, mesh, QString(), parent);
}

VasnecovProduct *ProductModel::addAssembly(const QString &name, VasnecovProduct *parent)
{
    if(hasWorkModel())
    {
        VasnecovProduct *product(nullptr);
        QString fullName = model()->name() + " " + name;
        if(!parent)
        {
            parent = model();
        }

        product = universe()->addAssembly(fullName, world(), parent);

        if(product)
        {
            return product;
        }
    }

    BMCL_WARNING() << "Can't create child assembly of 3D-model: " << name;
    return nullptr;
}

VasnecovLabel *ProductModel::addLabel(const QString &name, GLfloat width, GLfloat height, const QString &texture, const VasnecovAbstractElement *element)
{
    if(hasWorkModel())
    {
        VasnecovLabel *label(nullptr);
        QString fullName = model()->name() + " " + name;

        if(!texture.isEmpty())
        {
            label = universe()->addLabel(fullName, world(), width, height, texture);
        }
        else
        {
            label = universe()->addLabel(fullName, world(), width, height);
        }

        if(label)
        {
            if(element)
            {
                label->attachToElement(element);
            }
            return label;
        }
    }

    BMCL_WARNING() << "Can't create label for 3D-model: " << name;
    return nullptr;
}

VasnecovLabel *ProductModel::addLabel(const QString &name, GLfloat width, GLfloat height, const VasnecovAbstractElement *element)
{
    return addLabel(name, width, height, QString(), element);
}

VasnecovFigure *ProductModel::addFigure(const QString &name, const VasnecovAbstractElement *element)
{
    if(hasWorkModel())
    {
        VasnecovFigure *figure(nullptr);
        QString fullName = model()->name() + " " + name;

        figure = universe()->addFigure(fullName, world());

        if(figure)
        {
            if(element)
            {
                figure->attachToElement(element);
            }
            return figure;
        }
    }

    BMCL_WARNING() << "Can't create figure for 3D-model: " << name;
    return nullptr;
}
}
