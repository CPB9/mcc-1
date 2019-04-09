#include "PlotTool.h"
#include "PlotWidget.h"
#include "PlotCurve.h"

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

PlotTool::~PlotTool()
{
}

PlotTool::PlotTool(mccuav::UavController* uavController, QWidget* parent /*= 0*/)
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

    _plot = new PlotWidget(uavController, this);

    connect(btnClear, &QPushButton::pressed, this, [this]() {_plot->clearCurves(); });
    mainLayout->addWidget(_plot);

    setLayout(mainLayout);

    _plot->setAutoUpdate(true);
}

QColor PlotTool::findFreeColor() const
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
    return colors[_plot->curves().size() % numColors];
}

void PlotTool::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat(mccuav::PlotData::mimeDataStr()))
        event->acceptProposedAction();
}

void PlotTool::dropEvent(QDropEvent *event)
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
    event->acceptProposedAction();
}
