#include "mcc/map/SimpleFlagLayer.h"
#include "mcc/map/MapRect.h"

#include <QPoint>
#include <QPointF>
#include <QSize>

#include <bmcl/Logging.h>

namespace mccmap {

constexpr Qt::KeyboardModifier hintModifierKey = Qt::ShiftModifier;

SimpleFlagLayer::SimpleFlagLayer(const MapRect* rect)
    : Layer(rect)
    , _activeFlag(bmcl::None)
    , _activeLine(bmcl::None)
    , _configActionName("Свойства")
    , _isMoving(false)
    , _hasMoved(false)
{
}

void SimpleFlagLayer::draw(QPainter* p) const
{
    const bmcl::Option<QPoint>& pos = mapRect()->cursorPosition();
    if (isActive() && (mapRect()->modifiers() & hintModifierKey) && pos.isSome() && !_isMoving) {
        drawModifierHint(pos.unwrap(), p);
    }
    drawFlags(p);
}

void SimpleFlagLayer::drawModifierHint(const QPoint& pos, QPainter* p) const
{
    (void)pos;
    (void)p;
}

bool SimpleFlagLayer::addFlagAt(const QPointF&)
{
    return false;
}

bool SimpleFlagLayer::insertFlagAt(std::size_t, const QPointF&)
{
    return false;
}

bmcl::Option<LineIndexAndPos> SimpleFlagLayer::lineAt(const QPointF& pos)
{
    return bmcl::None;
}

void SimpleFlagLayer::showFlagEditor(const QPoint&)
{}

void SimpleFlagLayer::finishMovingCurrent()
{}

void SimpleFlagLayer::drawFlags(QPainter* p) const
{
    (void)p;
}

bool SimpleFlagLayer::mouseDoubleClickEvent(const QPoint& pos)
{
    if (_activeFlag.isSome()) {
        showFlagEditor(pos);
        return true;
    }
    return false;
}

bool SimpleFlagLayer::mousePressEvent(const QPoint& pos)
{
    if (_activeFlag.isSome()) {
        _isMoving = startMovingCurrent();
        _hasMoved = false;
        return _isMoving;
    } else if (_activeLine.isSome()) {
        if (!insertFlagAt(_activeLine.unwrap().index + 1, _activeLine.unwrap().pos)) {
            return false;
        }
        auto idx = _activeLine.unwrap().index + 1;
        _activeFlag = idx;
        _lastActiveFlag = idx;
        _activeLine = bmcl::None;
        setCurrentActive(true);
        _isMoving = true;
        _hasMoved = true;
        return true;
    } else if (isActive() && (mapRect()->modifiers() & hintModifierKey)) {
        _isMoving = true;
        _hasMoved = true;
        return addFlagAt(pos);
    }
    _isMoving = false;
    _hasMoved = false;
    return false;
}

bool SimpleFlagLayer::mouseMoveEvent(const QPoint& oldPos, const QPoint& newPos)
{
    if (_activeFlag.isSome() && _isMoving) {
        QPoint delta = newPos - oldPos;
        _hasMoved = moveCurrentBy(delta);
        _isMoving = _hasMoved;
        return _isMoving;
    } else {
        auto newActive = flagAt(newPos);
        if (_activeFlag.isSome() && _activeFlag != newActive) {
            setCurrentActive(false);
        }
        _activeFlag = newActive;
        if (_activeFlag.isSome()) {
            _lastActiveFlag = _activeFlag.unwrap();
        }

        if (_activeFlag.isSome()) {
            _activeLine = bmcl::None;
            if (isActive() && (mapRect()->modifiers() & hintModifierKey)) {
                setCurrentActive(false);
                _activeFlag = bmcl::None;
                return false;
            } else {
                setCurrentActive(true);
                return true;
            }
        } else {
            _activeLine = lineAt(newPos);
            return _activeLine.isSome();
        }
    }
    return false;
}

bool SimpleFlagLayer::onLeave()
{
    bool accepted = false;
    if (_activeFlag.isSome()) {
        if (_hasMoved) {
            finishMovingCurrent();
        }
        if (isActive() && (mapRect()->modifiers() & hintModifierKey)) {
            setCurrentActive(false);
            _activeFlag = bmcl::None;
        }
        accepted = true;
    }
    _isMoving = false;
    _hasMoved = false;
    return accepted;
}

void SimpleFlagLayer::mouseLeaveEvent()
{
    onLeave();
}

bool SimpleFlagLayer::mouseReleaseEvent(const QPoint& pos)
{
    Q_UNUSED(pos);
    return onLeave();
}

bool SimpleFlagLayer::startMovingCurrent()
{
    return true;
}

SimpleFlagLayer::~SimpleFlagLayer()
{
}
}
