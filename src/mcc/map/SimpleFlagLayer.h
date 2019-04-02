#pragma once

#include "mcc/Config.h"
#include "mcc/map/Layer.h"
#include "mcc/map/LineAndPos.h"

#include <bmcl/Option.h>

#include <QPointF>
#include <QString>

class QSize;
class QPoint;

namespace mccmap {

class MCC_MAP_DECLSPEC SimpleFlagLayer : public Layer {
    Q_OBJECT
public:
    explicit SimpleFlagLayer(const MapRect* rect);
    ~SimpleFlagLayer();
    void draw(QPainter* p) const override;
    void mouseLeaveEvent() override;
    bool mouseMoveEvent(const QPoint& oldPos, const QPoint& newPos) override;
    bool mousePressEvent(const QPoint& pos) override;
    bool mouseReleaseEvent(const QPoint& pos) override;
    bool mouseDoubleClickEvent(const QPoint & pos) override;


    inline const bmcl::Option<std::size_t>& activeFlag() const;
    inline const bmcl::Option<LineIndexAndPos>& activeLine() const;

protected:
    virtual bmcl::Option<std::size_t> flagAt(const QPointF& pos) = 0;
    virtual bool moveCurrentBy(const QPointF& delta) = 0;
    virtual void setCurrentActive(bool isActive) = 0;

    virtual bool startMovingCurrent();
    virtual void drawFlags(QPainter* p) const;
    virtual void drawModifierHint(const QPoint& pos, QPainter* p) const;
    virtual bool addFlagAt(const QPointF& pos);
    virtual bool insertFlagAt(std::size_t index, const QPointF& pos);
    virtual bmcl::Option<LineIndexAndPos> lineAt(const QPointF& pos);
    virtual void showFlagEditor(const QPoint& pos);
    virtual void finishMovingCurrent();

    bmcl::Option<std::size_t> _activeFlag;
    bmcl::Option<std::size_t> _lastActiveFlag;
    bmcl::Option<LineIndexAndPos> _activeLine;

    QString _configActionName;

private:
    bool onLeave();

    bool _isMoving;
    bool _hasMoved;
};

inline const bmcl::Option<std::size_t>& SimpleFlagLayer::activeFlag() const
{
    return _activeFlag;
}

inline const bmcl::Option<LineIndexAndPos>& SimpleFlagLayer::activeLine() const
{
    return _activeLine;
}
}
