#include "mcc/map/LayerWidget.h"
#include "mcc/map/LayerModel.h"
#include "mcc/map/LayerGroup.h"
#include "mcc/res/Resource.h"

#include <QTableView>
#include <QHBoxLayout>
#include <QPushButton>
#include <QHeaderView>
#include <QSortFilterProxyModel>
#include <QItemDelegate>
#include <QPaintEvent>
#include <QPainter>
#include <QString>

namespace mccmap {

class CheckBoxDelegate : public QItemDelegate
{
public:
    CheckBoxDelegate(const QPixmap& checked, const QPixmap& unchecked, QObject* parent = nullptr)
        : QItemDelegate(parent)
        , _checked(checked)
        , _unchecked(unchecked)
    {
    }

    ~CheckBoxDelegate()
    {
    }

    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override
    {
        if (event->type() == QEvent::MouseButtonRelease)
        {
            bool value = model->data(index, Qt::CheckStateRole).toBool();
            Qt::CheckState state = (!value) ? Qt::Checked : Qt::Unchecked;
            model->setData(index, state, Qt::CheckStateRole);
        }
        return true;
    }

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override
    {
        const QPixmap& pm = index.model()->data(index, Qt::CheckStateRole).toBool() ? _checked : _unchecked;
        auto location = option.rect.center() - pm.rect().center();

        painter->drawPixmap(location, pm);
    }
private:
    const QPixmap& _checked;
    const QPixmap& _unchecked;
};


class LayerProxyModel : public QSortFilterProxyModel
{
public:
    explicit LayerProxyModel(LayerModel* model)
    {
        setSourceModel(model);
    }

    virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override
    {
        QModelIndex srcIndex = sourceModel()->index(source_row, 3, source_parent);
        QString name = srcIndex.data(Qt::DisplayRole).toString();
        return !(name == "Карта" || name == "Линейка" || name == "KML");
    }
};

LayerWidget::LayerWidget(LayerGroup* layers, QWidget* parent)
    : QWidget(parent)
    , _layers(layers)
{
    _model = new LayerModel(layers);
    _proxyModel = new LayerProxyModel(_model);

    _view = new QTableView;
    _view->setModel(_proxyModel);
    _view->setSelectionMode(QAbstractItemView::SingleSelection);
    _view->setSelectionBehavior(QAbstractItemView::SelectRows);
    _view->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    _view->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    _view->verticalHeader()->setDefaultSectionSize(_view->verticalHeader()->minimumSectionSize());
    _view->verticalHeader()->hide();
    _view->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

    auto layout = new QHBoxLayout;
    layout->setContentsMargins(1, 1, 1, 1);
    layout->addWidget(_view);
    setLayout(layout);

    connect(_view, &QTableView::doubleClicked, this, [this](const QModelIndex& index) { _model->setActiveLayer(_proxyModel->mapToSource(index)); });

    _visibleRenderer = QPixmap::fromImage(mccres::renderSvg(":resources/eye_open.svg", 18, 18));
    _invisibleRenderer = QPixmap::fromImage(mccres::renderSvg(":resources/eye_closed.svg", 18, 18));
    _unlockedRenderer = QPixmap::fromImage(mccres::renderSvg(":resources/lock_open.svg", 18, 18));
    _lockedRenderer = QPixmap::fromImage(mccres::renderSvg(":resources/lock_closed.svg", 18, 18));

    _eyeDelegate = new CheckBoxDelegate(_visibleRenderer, _invisibleRenderer);
    _lockDelegate = new CheckBoxDelegate(_unlockedRenderer, _lockedRenderer);
    _view->setItemDelegateForColumn(0, _eyeDelegate);
    _view->setItemDelegateForColumn(1, _lockDelegate);
}

LayerWidget::~LayerWidget()
{
    delete _proxyModel;
    delete _model;
    delete _eyeDelegate;
    delete _lockDelegate;
}
}
