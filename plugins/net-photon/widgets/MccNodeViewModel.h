#pragma once

#include <photon/ui/QNodeViewModel.h>

#include <QMimeData>
#include "mcc/uav/PlotData.h"
#include "mcc/uav/Fwd.h"
#include <bmcl/Logging.h>

namespace mccphoton {

class MccNodeViewModel : public ::photon::QNodeViewModel
{
    Q_OBJECT
public:
    explicit MccNodeViewModel(::photon::NodeView* node)
        : ::photon::QNodeViewModel(node)
    {

    }

    virtual QStringList mimeTypes() const override { return { mccuav::PlotData::mimeDataStr() }; }
    virtual QMimeData * mimeData(const QModelIndexList &indexes) const override
    {
        for (const auto& i : indexes)
        {
            auto nodeView = (::photon::NodeView*)i.internalPointer();
            std::string trait;
            bmcl::OptionPtr<::photon::NodeView> current = nodeView->parent();
            while(current.isSome())
            {
                if (!trait.empty())
                {
                    trait = "." + trait;
                }
                trait = current->fieldName().toStdString() + trait;
                current = current->parent();
            }
            if (trait.size() > 4 && trait.substr(0, 4) == "cvm.")
                trait.erase(trait.begin(), trait.begin() + 4);
            else
            {
                BMCL_DEBUG() << "can't copy: not a simple variable";
                return new QMimeData();
            }

            std::string var = nodeView->fieldName().toStdString();

            mccuav::PlotData* plotData = new mccuav::PlotData(_device
                                                                        , trait
                                                                        , var
                                                                        , trait + "." + var);

            return mccuav::PlotData::packMimeData(plotData, mccuav::PlotData::mimeDataStr());
        }

        //BMCL_ASSERT(false);
        return new QMimeData();
    }
    virtual Qt::DropActions supportedDropActions() const override { return Qt::IgnoreAction; }
    virtual Qt::DropActions supportedDragActions() const override { return Qt::CopyAction; }

public slots:
    void setDevice(const mccmsg::Device& device) { _device = device; }

private:
    mccmsg::Device _device;
};
}
