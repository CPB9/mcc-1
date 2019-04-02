#pragma once

#include <vector>
#include <qwt_plot_curve.h>

class QwtSymbol;

class RouteCurve : public QwtPlotCurve
{
public:
    RouteCurve();
    ~RouteCurve();

    void setLoop(bool isLoop);
    void drawSymbols(QPainter *painter, const QwtSymbol& symbol, const QwtScaleMap& xMap, const QwtScaleMap& yMap, const QRectF& canvasRect, int from, int to) const override;
    void setActiveIndex(int idx);
    void clearSelection();
    void setSelectedIndices(const std::vector<size_t>& indices);
    const std::vector<size_t>& selectedIndices() const;

private:
    QwtSymbol* symbolOrdinary;
    QwtSymbol* symbolSelected;
    QwtSymbol* symbolActive;
    QwtSymbol* symbolActiveAndSelected;

    int _activeIndex;
    std::vector<size_t> _selectedIndices;
    bool _isLoop;
};
