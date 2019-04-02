#pragma once

#include <QStyledItemDelegate>

#include <QVBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QPainter>
#include <QMap>

#include <bmcl/Logging.h>

#include "AirframesModel.h"

namespace mccmav {

class AirframeViewDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    AirframeViewDelegate(QObject* parent)
        : QStyledItemDelegate(parent)
    {

    }


    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        return QSize(200, 170);
    }


    virtual QWidget * createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        QWidget* container = new QWidget(parent);
        QVBoxLayout* layout = new QVBoxLayout();
        container->setLayout(layout);

        auto groupLabel = new QLabel(index.data(AirframesModel::GroupNameRole).toString(), container);
        groupLabel->setAlignment(Qt::AlignCenter);

        layout->addWidget(groupLabel);

        auto imgLabel = new QLabel(container);
        imgLabel->setAlignment(Qt::AlignCenter);
        imgLabel->setPixmap(index.data(AirframesModel::GroupImageRole).value<QPixmap>());
        layout->addWidget(imgLabel);

        QComboBox* airframesWidget = new QComboBox(container);
        airframesWidget->setMaximumWidth(185);
        auto ids = index.data(AirframesModel::AirframesMap).value<QMap<int, QString>>();
        for (auto i : ids.keys())
        {
            airframesWidget->addItem(ids[i], i);
        }
        layout->addWidget(airframesWidget);

        airframesWidget->setProperty("modelIndex", index);
        connect(airframesWidget, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &AirframeViewDelegate::emitForceSelection);
        connect(this, &AirframeViewDelegate::updateSelection, this,
                [this, airframesWidget](const QModelIndex& index) {
                    const_cast<AirframeViewDelegate*>(this)->handleUpdateSelection(airframesWidget, index);
                }
        );

        return container;
    }

    void emitForceSelection()
    {
        QComboBox* widget = static_cast<QComboBox*>(sender());
        emit forceSelection(widget->property("modelIndex").value<QModelIndex>());
        emit airframeSelected(widget->currentData().toUInt());
    }

    void handleUpdateSelection(QComboBox* combo, const QModelIndex& index)
    {
         if (combo->property("modelIndex").value<QModelIndex>() != index)
             return;
         emit airframeSelected(combo->currentData().toUInt());
    }

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);

        painter->save();
        painter->fillRect(opt.rect, index.data(Qt::BackgroundRole).value<QColor>());
        if (option.state & QStyle::State_Selected) {
            painter->fillRect(opt.rect, opt.palette.highlight());
        }
        painter->drawRect(opt.rect);
        painter->restore();
    }

signals:
    void forceSelection(const QModelIndex& index);
    void airframeSelected(int aiframeId);
    void updateSelection(const QModelIndex& index);
};
}
