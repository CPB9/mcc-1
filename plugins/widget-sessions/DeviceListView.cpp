#include "DeviceListView.h"
#include "DeviceItem.h"

#include "mcc/ide/toolbar/AddEntityWidget.h"
#include "mcc/ide/toolbar/MainToolBar.h"

#include <QScrollArea>
#include <QVBoxLayout>
#include <QLabel>

DeviceListView::DeviceListView(const mccui::Rc<mccuav::UavController>& uavController)
    : _controller(uavController)
{
    connect(uavController.get(), &mccuav::UavController::uavAdded, this, &DeviceListView::onUavAdded);
    connect(uavController.get(), &mccuav::UavController::uavRemoved, this, &DeviceListView::onUavRemoved);

    auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->setContentsMargins(0, 0, 0, 0);
}

bool DeviceListView::empty() const
{
    return _widgets.empty();
}

void DeviceListView::onUavAdded(mccuav::Uav* uav)
{
    auto it = std::find_if(_widgets.begin(), _widgets.end(), [uav](DeviceItem* item) { return item->uav() == uav; });
    if(it != _widgets.end())
        return;

    auto w = new DeviceItem(_controller.get(), uav, this);
    _widgets.push_back(w);
    qobject_cast<QVBoxLayout*>(layout())->addWidget(w);
}

void DeviceListView::onUavRemoved(mccuav::Uav* uav)
{
    auto it = std::find_if(_widgets.begin(), _widgets.end(), [uav](DeviceItem* item) { return item->uav() == uav; });
    if(it == _widgets.end())
        return;
    delete *it;
}

void DeviceListView::onUavUpdated(mccuav::Uav* uav)
{

}

void DeviceListView::showEvent(QShowEvent *event)
{
//     int width(0);
//     int height(mccide::MainToolBar::blockMinimumSize().height());
//     for(const auto& w : _widgets)
//     {
//         height += mccide::MainToolBar::blockMinimumSize().height() + 3;
//         if(w->minimumWidth() > width)
//             width = w->minimumWidth();
//     }
//     resize(width, height + 2);
// 
//     _view->widget()->setFixedSize(width, height);
//     _view->setFixedSize(width, height + 2);
     QWidget::showEvent(event);
}
