#pragma once

#include "mcc/Config.h"

#include <deque>
#include <vector>
#include <cmath>

#include <QPixmap>

class QPainter;

namespace mccmap {

struct TilePosition;

class MCC_MAP_DECLSPEC MemoryCache {
public:
    MemoryCache();
    ~MemoryCache();

    inline int zoomLevel() const;
    const QPixmap& pixmapAt(int x, int y) const;
    inline int globalOffsetX(int offset = 0) const;
    inline int globalOffsetY(int offset = 0) const;
    inline QPoint globalOffset() const;
    inline int width() const; //FIXME: size_t
    inline int height() const; //FIXME: size_t
    inline int maxSize() const; //FIXME: size_t
    inline bool isAtTop() const;
    inline bool isAtBottom() const;

    inline void setDefaultPixmap(const QPixmap& p);

    void draw(QPainter* p) const;
    void drawNonTiled(QPainter* p) const;
    void updatePixmap(const mccmap::TilePosition& pos, const QPixmap& image);
    inline void resetPixmap(const mccmap::TilePosition& pos);

    std::vector<TilePosition> setPosition(int zoomLevel, int globalOffsetX, int globalOffsetY);
    std::vector<TilePosition> setOffset(int globalOffsetX, int globalOffsetY);
    std::vector<TilePosition> resize(int tileCountX, int tileCountY);
    std::vector<TilePosition> scrollUp();
    std::vector<TilePosition> scrollDown();
    std::vector<TilePosition> scrollLeft();
    std::vector<TilePosition> scrollRight();
    std::vector<TilePosition> reloadCache();

private:
    std::vector<TilePosition> setSize(int tileCountX, int tileCountY);
    int absOffset(int globalOffset) const;
    void loadRow(int row, std::vector<TilePosition>* queue);
    void loadColumn(int column, std::vector<TilePosition>* queue);
    void populateEmptyRow(std::deque<QPixmap>& row);
    inline void updateMaxSize();

    QPixmap _emptyPixmap;
    std::deque<std::deque<QPixmap>> _cache;
    int _zoomLevel;
    int _width;
    int _height;
    int _maxWidth;
    int _maxHeight;
    int _globalOffsetX;
    int _globalOffsetY;
    int _maxSize;
};

inline int MemoryCache::zoomLevel() const
{
    return _zoomLevel;
}

inline void MemoryCache::setDefaultPixmap(const QPixmap& p)
{
    _emptyPixmap = p;
}

inline int MemoryCache::maxSize() const
{
    return _maxSize;
}

inline void MemoryCache::resetPixmap(const TilePosition& pos)
{
    updatePixmap(pos, _emptyPixmap);
}

inline bool MemoryCache::isAtBottom() const
{
    return _globalOffsetY + _height >= _maxSize;
}

inline bool MemoryCache::isAtTop() const
{
    return _globalOffsetY <= 0;
}

inline int MemoryCache::height() const
{
    return _height;
}

inline int MemoryCache::width() const
{
    return _width;
}

inline int MemoryCache::globalOffsetX(int offset) const
{
    return absOffset(_globalOffsetX + offset);
}

inline int MemoryCache::globalOffsetY(int offset) const
{
    return absOffset(_globalOffsetY + offset);
}

inline QPoint MemoryCache::globalOffset() const
{
    return QPoint(_globalOffsetX, _globalOffsetY);
}

inline void MemoryCache::updateMaxSize()
{
    _maxSize = std::exp2(_zoomLevel);
}
}
