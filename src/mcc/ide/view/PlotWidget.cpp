#include "mcc/ide/view/PlotWidget.h"
#include "mcc/ide/view/MccPlot.h"

#include "mcc/uav/UavController.h"
#include "mcc/uav/Uav.h"
#include "mcc/uav/PlotData.h"

#include "mcc/msg/ParamList.h"

#include <bmcl/StringView.h>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QListView>
#include <QStringListModel>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>

namespace mccide {

PlotWidget::~PlotWidget()
{
}

PlotWidget::PlotWidget(mccuav::UavController* uavController, QWidget* parent /*= 0*/)
    : QWidget(parent)
    , _uavController(uavController)
{
    setObjectName("Графики");
    setWindowTitle("Графики");
    setWindowIcon(QIcon(":/graphs-icon.png"));

    setAcceptDrops(true);

    QVBoxLayout* mainLayout = new QVBoxLayout();
    QPushButton* btnClear = new QPushButton("Clear");
    mainLayout->addWidget(btnClear);

    _plot = new MccPlot(uavController, this);

    connect(btnClear, &QPushButton::pressed, this, [this]() {_plot->clearCurves(); });
    mainLayout->addWidget(_plot);

    setLayout(mainLayout);

    _plot->setAutoUpdate(true);
}

// void PlotWidget::tmParamList(const mccmsg::TmParamListPtr& params)
// {
//     auto curves = _plot->curves();
//     if (curves.empty())
//         return;
// 
//     for (auto c : curves)
//     {
//         auto device = c->variable().device();
//         if (device != params->device())
//             continue;
// 
//         auto p = params->findParam(c->variable().trait(), c->variable().varId());
//         if (p.isSome())
//         {
//             quint64 msecsToNow = bmcl::toMsecs(params->time().time_since_epoch()).count();
// 
//             _plot->curveValueChanged(c->name(), (double)msecsToNow, p->value().toDouble());
//         }
//     }
// }

QColor PlotWidget::findFreeColor() const
{
    const char *colors[] =
    {
        "Red",
        "Blue",
        "Green",
        "Purple",
        "Orange",
        "Aqua",
        "#ff78ff",
        "#7363ff",
        "#3cff36",
        "#220e8a"
    };

    const int numColors = sizeof(colors) / sizeof(colors[0]);
    auto isColorInUse = [this](const QColor& c)
    {
        for (auto it : _plot->curves())
        {
            if (it->color() == c)
                return true;
        }
        return false;
    };

    for (int i = 0; i < numColors; ++i)
    {
        if (!isColorInUse(colors[i]))
            return colors[i];
    }
    return colors[_plot->curves().count() % numColors];
}

void PlotWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat(mccuav::PlotData::mimeDataStr()))
        event->acceptProposedAction();
}

void PlotWidget::dropEvent(QDropEvent *event)
{
    auto plotData = mccuav::PlotData::unpackMimeData(event->mimeData(), mccuav::PlotData::mimeDataStr());
    if (plotData.isNone())
    {
        BMCL_ASSERT(false);
        return;
    }

    _plot->addCurve(QString::fromStdString((*plotData)->varId()),
                    QString::fromStdString((*plotData)->description()),
                    findFreeColor(),
                    *plotData.unwrap());

    _uavController->sendCmdYY(new mccmsg::CmdParamRead((*plotData)->device(), (*plotData)->trait(), { (*plotData)->varId() }));
    event->acceptProposedAction();
}
}
