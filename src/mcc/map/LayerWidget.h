#pragma once

#include "mcc/Config.h"
#include "mcc/map/Rc.h"

#include <QWidget>

class QTableView;
class QAbstractItemDelegate;

namespace mccmap {

class LayerModel;
class LayerGroup;
class LayerProxyModel;

class MCC_MAP_DECLSPEC LayerWidget : public QWidget {
    Q_OBJECT
public:
    LayerWidget(LayerGroup* layers, QWidget* parent = nullptr);
    ~LayerWidget();

private:
    Rc<LayerGroup> _layers;
    LayerModel* _model;
    LayerProxyModel* _proxyModel;
    QTableView* _view;
    QPixmap _visibleRenderer;
    QPixmap _invisibleRenderer;
    QPixmap _unlockedRenderer;
    QPixmap _lockedRenderer;
    QAbstractItemDelegate* _eyeDelegate;
    QAbstractItemDelegate* _lockDelegate;
};
}
