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
    saveState();
}

PlotTool::PlotTool(mccui::Settings* settings, mccuav::UavController* uavController, mccuav::PlotController* plotController, QWidget* parent /*= 0*/)
    : QWidget(parent)
    , _settings(settings)
    , _uavController(uavController)
    , _plotController(plotController)
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

    connect(plotController, &mccuav::PlotController::plotAdded, this,
            [this](const mccuav::PlotData& plotData)
            {
                addCurve(plotData);
                emit _plotController->requestSubscription(plotData);
            }
    );

    connect(uavController, &mccuav::UavController::uavTmStorageUpdated, this,
            [this](mccuav::Uav* uav)
            {
                if(uav->tmStorage() == nullptr)
                    return;

                auto it = _pendingCurves.begin();
                while(it != _pendingCurves.end())
                {
                    if(it->device() == uav->device())
                    {
                        _plotController->addPlot(*it);
                        it = _pendingCurves.erase(it);
                    }
                    else
                        ++it;
                }
            }
    );
    connect(uavController, &mccuav::UavController::uavRemoved, this,
            [this](mccuav::Uav* uav)
            {
                std::vector<QString> toRemove;
                for(const auto& it : _plot->curves())
                {
                    if(it->variable().device() == uav->device())
                        toRemove.push_back(it->name());
                }
                for(const auto& n : toRemove)
                {
                    _plot->removeCurve(n);
                }
            }
    );

    connect(_plot, &PlotWidget::requestSave, this, &PlotTool::saveState);

    _pendingCurves = _settings->read("plot/curves").value<QList<mccuav::PlotData>>();
}

void PlotTool::saveState()
{
    QList<mccuav::PlotData> cfg;
    cfg.reserve(_plot->curves().size());
    for(const auto& c : _plot->curves())
    {
        cfg.push_back(c->variable());
    }
    _settings->tryWrite("plot/curves", QVariant::fromValue(cfg));
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

void PlotTool::addCurve(const mccuav::PlotData& data)
{
    const auto& curves = _plot->curves();
    auto it = std::find_if(curves.begin(), curves.end(), [data](const Curve* c) {return c->variable() == data; });
    if(it != curves.end())
        return;
    _plot->addCurve(QString::fromStdString(data.varId()),
                    QString::fromStdString(data.description()),
                    findFreeColor(),
                    data);
    saveState();
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

    addCurve(*plotData.take());
    event->acceptProposedAction();
}
