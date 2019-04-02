#pragma once

#include "mcc/3d/BasicModels.h"

class QSize;

namespace mcc3d
{
class MCC_3D_DECLSPEC TextInformer : public SimplestModel
{
public:
    TextInformer(VasnecovUniverse *u, VasnecovWorld *w, const QSize& labelSize);
    virtual ~TextInformer();

    virtual void setVisible(bool visible);
    bool isVisible() const {return _isVisible;}
    virtual void setColor(const QColor &c);
    QColor color() const {return _color;}
    virtual void setCoordinates(const QVector3D& coordinates);
    QVector3D coordinates() const;
    void setRounded(bool rounded);
    bool isRounded() const {return _isRounded;}

    void setText(const QString& text);

protected:
    virtual void generateText();
    virtual void redrawText();
    void drawText(bool bold = false, int size = 0);

    VasnecovLabel*  _label;
    QColor          _color;
    bool            _isVisible;
    QString         _valueText;
    bool            _isRounded;

private:
    Q_DISABLE_COPY(TextInformer)
};

class MCC_3D_DECLSPEC NumericalInformer : public TextInformer
{
public:
    NumericalInformer(VasnecovUniverse *u, VasnecovWorld *w, const QString &suffix);
    ~NumericalInformer() override;

    void setValue(float value);
    void setPrecision(int precision);
    int precision() const {return _precision;}
    float value() const {return _value;}

protected:
    void generateText() override;

    QString         _suffix;
    float           _value;
    int             _precision;
private:
    void setText(const QString& text);

    Q_DISABLE_COPY(NumericalInformer)
};

class MCC_3D_DECLSPEC DistanceInformer : public NumericalInformer
{
public:
    DistanceInformer(VasnecovUniverse *u, VasnecovWorld *w, const QString &postfix);
    ~DistanceInformer() override;

    void setVisible(bool visible) override;
    void setColor(const QColor &c) override;
    void setPoints(const QVector3D &first, const QVector3D &second, const QColor &c = QColor());
    float length() const {return _value;}

protected:
    void generateText() override;
    void redrawText() override;

private:
    VasnecovFigure* _line;
    QString         _postfix;

    Q_DISABLE_COPY(DistanceInformer)
};
}
