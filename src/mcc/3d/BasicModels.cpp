#include "mcc/3d/BasicModels.h"
#include <QApplication>
#include <VasnecovUniverse>
#include <VasnecovFigure>
#include <VasnecovProduct>
#include <VasnecovLabel>

#include <cmath>
#include <cassert>

#include <bmcl/Math.h>

namespace mcc3d {

SimplestModel::SimplestModel(VasnecovUniverse *universe, VasnecovWorld *world) :
    _universe(universe),
    _world(world)
{}

MetricsModel::MetricsModel(VasnecovUniverse *u, VasnecovWorld *w, const QString& texturePath) :
    SimplestModel(u, w),
    _labels()
{
    if(universe() != nullptr && world() != nullptr)
    {
        _labels.reserve(5 * 2 + 10 *2);

        // metrics
        VasnecovLabel *label(nullptr);

        for(int i = 0; i < 5; ++i)
        {
            label = universe()->addLabel("metric-10x", world(), 32, 16, texturePath);
            if(label)
            {
                label->setCoordinates(10 + 10 * i, 0, 0);
                label->setTextureZone(89, 16 * i, 32, 16);
                _labels.push_back(label);
            }

            label = universe()->addLabel("metric-10y", world(), 32, 16, texturePath);
            if(label)
            {
                label->setCoordinates(0, 10 + 10 * i, 0);
                label->setTextureZone(89, 16 * i, 32, 16);
                _labels.push_back(label);
            }
        }

        for(int i = 0; i < 9; ++i)
        {
            label = universe()->addLabel("metric-1x", world(), 24, 16, texturePath);
            if(label)
            {
                label->setCoordinates(1 + 1 * i, 0, 0);
                label->setTextureZone(52, 16 * i, 24, 16);
                _labels.push_back(label);
            }

            label = universe()->addLabel("metric-1y", world(), 24, 16, texturePath);
            if(label)
            {
                label->setCoordinates(0, 1 + 1 * i, 0);
                label->setTextureZone(52, 16 * i, 24, 16);
                _labels.push_back(label);
            }
        }
    }
}

MetricsModel::~MetricsModel()
{
    if(universe() != nullptr)
    {
        for(auto label : _labels)
        {
            universe()->removeLabel(label);
        }
        _labels.clear();
    }
}

AxisModel::AxisModel(VasnecovUniverse *u, VasnecovWorld *w, float axisLength, bool withArrows) :
    SimplestModel(u, w),
    _figures(),
    _products()
{
    if(universe() != nullptr && world() != nullptr)
    {
        if(withArrows)
        {
            VasnecovProduct *axis = universe()->addAssembly("Axis", world());
            if(axis)
            {
                _products.push_back(axis);

                axis->setScale(0.1f);

                // Child products will be removed by axis-product
                VasnecovProduct *axisX = universe()->addPart("axisX", world(), "axis_arrow", axis);
                if(axisX)
                {
                    axisX->setColor(0xFF0000);
                    axisX->setCoordinates(5, 0, 0);
                    axisX->setAngles(0, 90, 0);
                }
                VasnecovProduct *axisY = universe()->addPart("axisY", world(), "axis_arrow", axis);
                if(axisY)
                {
                    axisY->setColor(0x00FF00);
                    axisY->setCoordinates(0, 5, 0);
                    axisY->setAngles(90, 0, 180);
                }
                VasnecovProduct *axisZ = universe()->addPart("axisZ", world(), "axis_arrow", axis);
                if(axisZ)
                {
                    axisZ->setColor(0x0000FF);
                    axisZ->setCoordinates(0, 0, 5);
                }

                VasnecovProduct *ball = universe()->addPart("axisBall", world(), "axis_ball", axis);
                if(ball)
                {
                    ball->setColor(0xFFFF00);
                }
            }
        }

        // Lines
        VasnecovFigure *figure = universe()->addFigure("aX", world());
        if(figure)
        {
            figure->createLine(QVector3D(0.0, 0.0, 0.0), QVector3D(axisLength, 0.0, 0.0), QColor(255, 0, 0, 255));
            figure->setScale(0.1f);
            figure->setThickness(2);
            _figures.push_back(figure);
        }

        figure = universe()->addFigure("aX", world());
        if(figure)
        {
            figure->createLine(QVector3D(0.0, 0.0, 0.0), QVector3D(0.0, axisLength, 0.0), QColor(0, 255, 0, 255));
            figure->setScale(0.1f);
            figure->setThickness(2);
            _figures.push_back(figure);
        }

        figure = universe()->addFigure("aX", world());
        if(figure)
        {
            figure->createLine(QVector3D(0.0, 0.0, 0.0), QVector3D(0.0, 0.0, axisLength), QColor(0, 0, 255, 255));
            figure->setScale(0.1f);
            figure->setThickness(2);
            _figures.push_back(figure);
        }
    }
}

AxisModel::~AxisModel()
{
    if(universe() != nullptr)
    {
        for(auto figure : _figures)
        {
            universe()->removeFigure(figure);
        }
        _figures.clear();

        for(auto product : _products)
        {
            universe()->removeProduct(product);
        }
        _products.clear();
    }
}

void AxisModel::setOverpainting(bool overpainting)
{
    for(auto figure : _figures)
    {
        figure->setDepth(!overpainting);
    }
}

void AxisModel::setLengthX(float length, bool doubleDirection)
{
    _figures[0]->createLine(QVector3D(doubleDirection ? -length : 0.0, 0.0, 0.0), QVector3D(length, 0.0, 0.0), QColor(255, 0, 0, 255));
}

void AxisModel::setLengthY(float length, bool doubleDirection)
{
    _figures[1]->createLine(QVector3D(0.0, doubleDirection ? -length : 0.0, 0.0), QVector3D(0.0, length, 0.0), QColor(0, 255, 0, 255));
}

void AxisModel::setLengthZ(float length, bool doubleDirection)
{
    _figures[2]->createLine(QVector3D(0.0, 0.0, doubleDirection ? -length : 0.0), QVector3D(0.0, 0.0, length), QColor(0, 0, 255, 255));
}

CompasModel::CompasModel(VasnecovUniverse *u, VasnecovWorld *w, float maxRadius, float minRadius, const QString& texturePath) :
    SimplestModel(u, w),
    _figures(),
    _labels(),
    _isVisible(true)
{
    if(universe() && world())
    {
        _figures.reserve(2);

        VasnecovFigure *figure = universe()->addFigure("Circle0", world());
        if(figure)
        {
            figure->createCircle(maxRadius, QColor(66, 63, 61, 200), 128);
            _figures.push_back(figure);
        }

        figure = universe()->addFigure("Circle1", world());
        if(figure)
        {
            figure->createCircle(minRadius, QColor(66, 63, 61, 200), 128);
            _figures.push_back(figure);
        }


        _labels.reserve(4);
        float scaleFactor = qApp->devicePixelRatio();
        VasnecovLabel *label = universe()->addLabel("N", world(), 15 * scaleFactor, 15 * scaleFactor, texturePath);
        if(label)
        {
            label->setCoordinates(0, minRadius + (maxRadius - minRadius)/2.0f, 0);
            label->setTextureZone(0, 0, 15, 15);
            _labels.push_back(label);
        }
        label = universe()->addLabel("E", world(), 16 * scaleFactor, 16 * scaleFactor, texturePath);
        if(label)
        {
            label->setCoordinates((minRadius + (maxRadius - minRadius)/2.0f), 0, 0);
            label->setTextureZone(0, 32, 16, 16);
            _labels.push_back(label);
        }
        label = universe()->addLabel("S", world(), 16 * scaleFactor, 16 * scaleFactor, texturePath);
        if(label)
        {
            label->setCoordinates(0, - (minRadius + (maxRadius - minRadius)/2.0f), 0);
            label->setTextureZone(0, 16, 16, 16);
            _labels.push_back(label);
        }
        label = universe()->addLabel("W", world(), 16 * scaleFactor, 16 * scaleFactor, texturePath);
        if(label)
        {
            label->setCoordinates(-(minRadius + (maxRadius - minRadius)/2.0f), 0, 0);
            label->setTextureZone(0, 48, 16, 16);
            _labels.push_back(label);
        }
    }
}

CompasModel::~CompasModel()
{
    if(universe())
    {
        for(auto figure : _figures)
        {
            universe()->removeFigure(figure);
        }
        _figures.clear();

        for(auto label : _labels)
        {
            universe()->removeLabel(label);
        }
        _labels.clear();
    }
}

void CompasModel::setColor(const QColor &color)
{
    for(VasnecovFigure* figure : _figures)
    {
        figure->setColor(color);
    }
}

void CompasModel::setThickness(float thickness)
{
    for(VasnecovFigure* figure : _figures)
    {
        figure->setThickness(thickness);
    }
}

void CompasModel::setVisible(bool visible)
{
    if(_isVisible == visible)
        return;

    _isVisible = visible;

    for(auto figure : _figures)
        figure->setVisible(visible);

    for(auto label : _labels)
        label->setVisible(visible);
}

void CompasModel::setOverpainting(bool overpainting)
{
    for(auto figure : _figures)
    {
        figure->setDepth(!overpainting);
    }
}

void CompasModel::updateScaleFromCamera()
{
    if(world() == nullptr)
        return;

    const float factor(0.25f);
    float r = (world()->cameraPosition() - world()->cameraTarget()).length();
    for(size_t i = 0; i < _labels.size(); ++i)
    {
        float deg = 90 * i;
        _labels[i]->setCoordinates(r * factor * std::sin(bmcl::degreesToRadians(deg)),
                                   r * factor * std::cos(bmcl::degreesToRadians(deg)),
                                   0.0f);
    }

    for(auto f : _figures)
    {
        f->setScale(r*0.0001f * factor);
    }
}

}
