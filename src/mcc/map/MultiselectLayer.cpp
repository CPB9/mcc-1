#include "mcc/map/MultiselectLayer.h"
#include "mcc/map/MapRect.h"

#include <QPoint>
#include <QSize>

#include <bmcl/Logging.h>

namespace mccmap {

constexpr Qt::KeyboardModifier hintModifierKey = Qt::ShiftModifier;
constexpr Qt::KeyboardModifier addModifierKey = Qt::ControlModifier;

MultiselectLayer::MultiselectLayer(const MapRect* rect)
    : Layer(rect)
    , _isMoving(false)
    , _hasMoved(false)
{
}

void MultiselectLayer::draw(QPainter* p) const
{
    const bmcl::Option<QPoint>& pos = mapRect()->cursorPosition();
    if (isActive() && (mapRect()->modifiers() & hintModifierKey) && pos.isSome() && !_isMoving) {
        drawModifierHint(pos.unwrap(), p);
    }
    drawElements(p);
}

void MultiselectLayer::drawModifierHint(const QPoint& pos, QPainter* p) const
{
    (void)pos;
    (void)p;
}

bmcl::Option<std::size_t> MultiselectLayer::appendElementAt(const QPoint&)
{
    return false;
}

bool MultiselectLayer::insertElementAt(std::size_t, const QPoint&)
{
    return false;
}

bmcl::Option<LineIndexAndPos> MultiselectLayer::lineAt(const QPoint&)
{
    return bmcl::None;
}

void MultiselectLayer::showEditor(const QPoint&)
{
}

void MultiselectLayer::finishMovingSelected()
{
}

void MultiselectLayer::drawElements(QPainter* p) const
{
    (void)p;
}

bool MultiselectLayer::mouseDoubleClickEvent(const QPoint& pos)
{
    return mousePressEvent(pos);
}

bool MultiselectLayer::mousePressEvent(const QPoint& pos)
{
    bmcl::Option<std::size_t> element = elementAt(pos);
    hover(element);
    if (element.isSome()) {
        std::size_t idx = element.unwrap();
        bool isSel = isSelected(idx);

        if (mapRect()->modifiers() & addModifierKey) {
            if (!isSel) {
                select(idx, true);
            } else {
                select(idx, false);
                _addSelectionOnMove.emplace(idx);
            }
        } else {
            if (!isSel) {
                selectAll(false);
                select(idx, true);
            } else {
                _selectSingleOnRelease.emplace(idx);
            }
        }

        if (hasSelected()) {
            _isMoving = startMovingSelected();
            _hasMoved = false;
            return _isMoving;
        }
    }

    selectAll(false);

    bmcl::Option<LineIndexAndPos> lineOpt = lineAt(pos);
    hoverLine(lineOpt);

    if (lineOpt.isSome()) {
        const LineIndexAndPos& line = lineOpt.unwrap();
        std::size_t idx = line.index + 1;
        if (!insertElementAt(idx, line.pos.toPoint())) {
            return false;
        }
        select(idx, true);
        hover(bmcl::Option<std::size_t>(idx));
        hoverLine(bmcl::None);
        _isMoving = true;
        _hasMoved = true;
        return true;
    } else if (isActive() && (mapRect()->modifiers() & hintModifierKey)) {
        bmcl::Option<std::size_t> appended = appendElementAt(pos);
        if (appended.isNone()) {
            return false;
        }
        std::size_t idx = appended.unwrap();
        hover(bmcl::Option<std::size_t>(idx));
        _isMoving = true;
        _hasMoved = true;
        return true;
    }
    _isMoving = false;
    _hasMoved = false;
    return false;
}

bool MultiselectLayer::mouseMoveEvent(const QPoint& oldPos, const QPoint& newPos)
{
    if (_addSelectionOnMove.isSome()) {
        std::size_t idx = _addSelectionOnMove.unwrap();
        select(idx, true);
        _addSelectionOnMove.clear();
    }

    if (hasSelected() && _isMoving) {
        QPoint delta = newPos - oldPos;
        _hasMoved = moveSelected(delta);
        _isMoving = _hasMoved;
        _selectSingleOnRelease.clear();
        return _isMoving;
    } else {
        bmcl::Option<std::size_t> element = elementAt(newPos);
        hover(element);

        if (element.isNone()) {
            bmcl::Option<LineIndexAndPos> line = lineAt(newPos);
            hoverLine(line);
            if (line.isNone()) {
                return false;
            }
        }
        return true;
    }
    return false;
}

bool MultiselectLayer::onLeave()
{
    bool accepted = false;
    if (hasSelected()) {
        if (_hasMoved) {
            finishMovingSelected();
        }
        if (isActive() && (mapRect()->modifiers() & hintModifierKey)) {
            hover(bmcl::None);
            hoverLine(bmcl::None);
        }
        accepted = true;
    }
    _isMoving = false;
    _hasMoved = false;
    return accepted;
}

void MultiselectLayer::mouseLeaveEvent()
{
    onLeave();
}

bool MultiselectLayer::mouseReleaseEvent(const QPoint& pos)
{
    Q_UNUSED(pos);
    _addSelectionOnMove.clear();
    if (_selectSingleOnRelease.isSome()) {
        selectAll(false);
        std::size_t idx = _selectSingleOnRelease.unwrap();
        select(idx, true);
        _selectSingleOnRelease.clear();
    }
    return onLeave();
}

bool MultiselectLayer::startMovingSelected()
{
    return true;
}

MultiselectLayer::~MultiselectLayer()
{
}

void MultiselectLayer::hover(const bmcl::Option<std::size_t>& index)
{
}

void MultiselectLayer::hoverLine(const bmcl::Option<LineIndexAndPos>& index)
{
}
}
