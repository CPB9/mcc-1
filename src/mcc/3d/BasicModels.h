#pragma once

#include "mcc/Config.h"

#include <vector>
#include <QColor>

class QVector3D;

class VasnecovUniverse;
class VasnecovWorld;

class VasnecovProduct;
class VasnecovFigure;
class VasnecovLabel;

namespace mcc3d {

class MCC_3D_DECLSPEC SimplestModel
{
public:
    SimplestModel(VasnecovUniverse *universe, VasnecovWorld *world);

protected:
    VasnecovUniverse *universe() {return _universe;}
    VasnecovWorld *world() {return _world;}

private:
    VasnecovUniverse *_universe;
    VasnecovWorld    *_world;
};

class MCC_3D_DECLSPEC MetricsModel : public SimplestModel
{
public:
    MetricsModel(VasnecovUniverse *universe,
                 VasnecovWorld *world);
    ~MetricsModel();

private:
    std::vector<VasnecovLabel *>  _labels;

    Q_DISABLE_COPY(MetricsModel)
};

class MCC_3D_DECLSPEC CompasModel : public SimplestModel
{
public:
    CompasModel(VasnecovUniverse *universe,
                VasnecovWorld *world,
                float maxRadius = 1.50f,
                float minRadius = 1.35f);
    ~CompasModel();
    bool isVisible() const {return _isVisible;}

    void setColor(const QColor &color);
    void setThickness(float thickness);
    void setVisible(bool visible);
    void setOverpainting(bool overpainting);
    void updateScaleFromCamera();

private:
    std::vector<VasnecovFigure *>   _figures;
    std::vector<VasnecovLabel *>    _labels;

    bool                            _isVisible;
    float                           _scale;

    Q_DISABLE_COPY(CompasModel)
};


class MCC_3D_DECLSPEC AxisModel : public SimplestModel
{
public:
    AxisModel(VasnecovUniverse *u, VasnecovWorld *w, float axisLength = 5.0f);
    ~AxisModel();
    void setOverpainting(bool overpainting);
    void setLengthX(float length, bool doubleDirection = false);
    void setLengthY(float length, bool doubleDirection = false);
    void setLengthZ(float length, bool doubleDirection = false);

private:
    std::vector<VasnecovFigure *>   _figures;
    std::vector<VasnecovProduct *>  _products;

    Q_DISABLE_COPY(AxisModel)
};


}
