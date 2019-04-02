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

class MCC_MAP_DECLSPEC MultiselectLayer : public Layer {
    Q_OBJECT
public:
    explicit MultiselectLayer(const MapRect* rect);
    ~MultiselectLayer();
    void draw(QPainter* p) const override;
    void mouseLeaveEvent() override;
    bool mouseMoveEvent(const QPoint& oldPos, const QPoint& newPos) override;
    bool mousePressEvent(const QPoint& pos) override;
    bool mouseReleaseEvent(const QPoint& pos) override;
    bool mouseDoubleClickEvent(const QPoint & pos) override;

protected:
    virtual bmcl::Option<std::size_t> elementAt(const QPoint& pos) = 0;
    virtual bool moveSelected(const QPoint& delta) = 0;
    virtual void select(std::size_t index, bool isSelected) = 0;
    virtual void selectAll(bool isSelected) = 0;
    virtual bool isSelected(std::size_t index) const = 0;
    virtual bool hasSelected() const = 0;

    virtual void hover(const bmcl::Option<std::size_t>& index);
    virtual void hoverLine(const bmcl::Option<LineIndexAndPos>& index);
    virtual bool startMovingSelected();
    virtual void finishMovingSelected();
    virtual void drawElements(QPainter* p) const;
    virtual void drawModifierHint(const QPoint& pos, QPainter* p) const;
    virtual bmcl::Option<std::size_t> appendElementAt(const QPoint& pos);
    virtual bool insertElementAt(std::size_t index, const QPoint& pos);
    virtual bmcl::Option<LineIndexAndPos> lineAt(const QPoint& pos);
    virtual void showEditor(const QPoint& pos);

private:
    bool onLeave();

    bmcl::Option<std::size_t> _selectSingleOnRelease;
    bmcl::Option<std::size_t> _addSelectionOnMove;
    bool _isMoving;
    bool _hasMoved;
};
}
