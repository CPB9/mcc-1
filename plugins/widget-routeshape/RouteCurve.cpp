#include "RouteCurve.h"

#include <qwt_symbol.h>
#include <qwt_point_mapper.h>
#include <qwt_painter.h>


RouteCurve::RouteCurve()
    : symbolOrdinary(new QwtSymbol(QwtSymbol::Ellipse, Qt::blue, QPen(Qt::black, 1), QSize(16, 16)))
    , symbolSelected(new QwtSymbol(QwtSymbol::Ellipse, Qt::yellow, QPen(Qt::black, 1), QSize(16, 16)))
    , symbolActive(new QwtSymbol(QwtSymbol::Ellipse, Qt::red, QPen(Qt::black, 1), QSize(16, 16)))
    , symbolActiveAndSelected(new QwtSymbol(QwtSymbol::Ellipse, Qt::magenta, QPen(Qt::black, 1), QSize(16, 16)))
    , _activeIndex(-1)
    , _isLoop(false)
{
}

RouteCurve::~RouteCurve()
{
    delete symbolOrdinary;
    delete symbolSelected;
    delete symbolActive;
    delete symbolActiveAndSelected;
}

void RouteCurve::setLoop(bool isLoop)
{
    _isLoop = isLoop;
}

void RouteCurve::drawSymbols(QPainter *painter, const QwtSymbol& symbol, const QwtScaleMap& xMap, const QwtScaleMap& yMap, const QRectF& canvasRect, int from, int to) const
{
    QwtPointMapper mapper;
    mapper.setFlag(QwtPointMapper::RoundPoints, QwtPainter::roundingAlignment(painter));
    mapper.setFlag(QwtPointMapper::WeedOutPoints, testPaintAttribute(QwtPlotCurve::FilterPoints));
    mapper.setBoundingRect(canvasRect);

    const int chunkSize = 500;

    auto isPointSelected = [this](size_t i)
    {
        return std::find(_selectedIndices.begin(), _selectedIndices.end(), i) != _selectedIndices.end();
    };

    for (int i = from; i <= to; i += chunkSize)
    {
        const int n = qMin(chunkSize, to - i + 1);

        const QPolygonF points = mapper.toPointsF(xMap, yMap, data(), i, i + n - 1);

        if (points.size() > 0)
        {
            //symbol.drawSymbols(painter, points);
        }

        for (int j = 0; j < points.size(); ++j)
        {
            QRectF rectf(points[j].x() - symbolOrdinary->size().width() / 2, points[j].y() - symbolOrdinary->size().height() / 2, symbolOrdinary->size().width(), symbolOrdinary->size().height());
            int wpIndex = j + from;
            if (_isLoop && wpIndex == this->data()->size() - 1)
                wpIndex = 0;

            int selectedIndex = isPointSelected(j) ? j : -1;
            if (j == selectedIndex && j == _activeIndex) {
                painter->setPen(Qt::black);
                symbolSelected->drawSymbol(painter, points[j]);
                QwtPainter::drawText(painter, rectf, Qt::AlignCenter, QString::number(wpIndex + 1));
                continue;
            }
            if (j == selectedIndex)
            {
                painter->setPen(Qt::black);
                symbolSelected->drawSymbol(painter, points[j]);
                QwtPainter::drawText(painter, rectf, Qt::AlignCenter, QString::number(wpIndex + 1));
                continue;
            }
            if (j == _activeIndex) {
                painter->setPen(Qt::black);
                symbolActive->drawSymbol(painter, points[j]);
                QwtPainter::drawText(painter, rectf, Qt::AlignCenter, QString::number(wpIndex + 1));
                continue;
            }

            painter->setPen(Qt::white);
            symbolOrdinary->drawSymbol(painter, points[j]);
            QwtPainter::drawText(painter, rectf, Qt::AlignCenter, QString::number(wpIndex + 1));
        }
    }
}

void RouteCurve::setActiveIndex(int idx)
{
    _activeIndex = idx;
}

void RouteCurve::clearSelection()
{
    _selectedIndices.clear();
}

void RouteCurve::setSelectedIndices(const std::vector<size_t>& indices)
{
    clearSelection();
    _selectedIndices = indices;
}

const std::vector<size_t>& RouteCurve::selectedIndices() const
{
    return _selectedIndices;
}
