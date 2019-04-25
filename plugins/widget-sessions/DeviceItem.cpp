#include "DeviceItem.h"

#include "mcc/uav/Uav.h"
#include "mcc/uav/UavController.h"
#include <QLabel>
#include <QHBoxLayout>
#include "mcc/ui/SliderCheckBox.h"

#include <bmcl/Logging.h>

DeviceItem::DeviceItem(mccuav::UavController* controller, mccuav::Uav* uav, QWidget* parent /*= nullptr*/)
    : QWidget(parent), _uav(uav)
{
    _name = new QLabel(this);
    _slider = new mccui::OnOffSliderCheckBox(_uav->device(), true, this);

    auto layout = new QHBoxLayout();
    setLayout(layout);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(_slider);
    layout->addWidget(_name);
    layout->addStretch();

    connect(_uav, &mccuav::Uav::deviceDescriptionUpdated, this, &DeviceItem::updateData);

    connect(_slider, &mccui::SliderCheckBox::sliderStateChanged, this, [this, controller](const bmcl::Uuid& uuid, bool checked)
            {
                controller->requestUavUpdate((mccmsg::Device)uuid, bmcl::None, bmcl::None, checked, bmcl::None);
            }
    );
    updateData();
}

mccuav::Uav* DeviceItem::uav() const
{
    return _uav;
}

void DeviceItem::updateData()
{
    assert(_uav);

    _name->setText(_uav->getInfo());
    _slider->setChecked(_uav->deviceDescription()->log());
}
