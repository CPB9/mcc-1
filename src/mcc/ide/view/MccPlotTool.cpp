#include "mcc/ide/view/MccPlotTool.h"

#include <QWidget>
#include <QStandardItemModel>
#include <QVBoxLayout>

#include "mcc/ide/view/MccPlot.h"
#include "mcc/msg/obj/Tm.h"

#include "mcc/uav/UavController.h"
#include "mcc/uav/Uav.h"
#include "mcc/ui/Settings.h"

#include "mcc/ui/TreeItem.h"
#include "mcc/ui/TraitsModel.h"

#include "mcc/ide/models/PlotTraitsModel.h"

namespace mccide {


struct PlotSettings {
    TraitDescription trait;
    QColor color;
    double multiplier;
    double offset;
};

typedef QVector<PlotSettings> PlotSettingsList;

/*
PlotSettingsList Settings::plotSettingsList()
{
    PlotSettingsList list;
    auto size = _settings.beginReadArray("ide/plotSettings");
    for (auto i = 0; i < size; ++i)
    {
        _settings.setArrayIndex(i);
        auto device = _settings.value("device").toString();
        auto trait = _settings.value("trait").toString();
        auto name = _settings.value("name").toString();
        auto color = _settings.value("color").value<QColor>();
        auto multiplier = _settings.value("multiplier").toDouble();
        auto offset = _settings.value("offset").toDouble();

        PlotSettings settings;
        settings.trait.device = device;
        settings.trait.trait = trait;
        settings.trait.name = name;
        settings.color = color;
        settings.multiplier = multiplier;
        settings.offset = offset;
        list.append(settings);
    }
    _settings.endArray();

    return list;
}

void Settings::setPlotSettingsList(const PlotSettingsList& plots)
{
    _settings.beginGroup("ide/plotSettings");
    _settings.remove("");
    _settings.endGroup();

    _settings.beginWriteArray("ide/plotSettings", plots.size());
    for (auto i = 0; i < plots.size(); ++i)
    {
        _settings.setArrayIndex(i);
        _settings.setValue("device", plots[i].trait.device);
        _settings.setValue("trait", plots[i].trait.trait);
        _settings.setValue("name", plots[i].trait.name);
        _settings.setValue("color", plots[i].color);
        _settings.setValue("multiplier", plots[i].multiplier);
        _settings.setValue("offset", plots[i].offset);
    }
    _settings.endArray();
}
*/

MccPlotWidget::MccPlotWidget(mccuav::UavController* uavController, QWidget* parent /*= 0*/)
    : QWidget(parent)
    , _uavController(uavController)
    , _isLoadingSettings(false)
{
    _ui.setupUi(this);

    setObjectName("MccPlotTool");
    setWindowTitle("Графики");

    _model = mccCtx->traitsModel().get();
    _proxyModel = new PlotTraitsModel(_model);

    _ui.firmwareTreeWidget->setUavController(_uavController.get());
    _ui.firmwareTreeWidget->setModel(_proxyModel);
    _ui.firmwareTreeWidget->setTraitsModel(_model);

     connect(_proxyModel, &PlotTraitsModel::tmParamSubscribeChanged, this, &MccPlotWidget::tmParamSubscribeChanged);

     connect(_uavController.get(), &mccuav::UavController::tmParamList, this, &MccPlotWidget::tmParamList);

     connect(_ui.groundTime, &QCheckBox::stateChanged, _ui.plot, &MccPlot::setAutoUpdate);

     _ui.timeInterval->addItem("10 секунд", 10);
     _ui.timeInterval->addItem("20 секунд", 20);
     _ui.timeInterval->addItem("30 секунд", 30);
     _ui.timeInterval->addItem("40 секунд", 40);
     _ui.timeInterval->addItem("50 секунд", 50);
     _ui.timeInterval->addItem("1 минута", 1 * 60);
     _ui.timeInterval->addItem("2 минуты", 2 * 60);
     _ui.timeInterval->addItem("3 минуты", 3 * 60);
     _ui.timeInterval->addItem("4 минуты", 4 * 60);
     _ui.timeInterval->addItem("5 минут", 5 * 60);
     _ui.timeInterval->addItem("10 минут", 10 * 60);

    connect(_ui.timeInterval, static_cast<void(QComboBox::*)(int index)>(&QComboBox::currentIndexChanged), this, &MccPlotWidget::timeIntervalChanged);
    connect(_ui.btnClear, &QPushButton::pressed, _proxyModel, &PlotTraitsModel::clearAllParams);

    connect(_uavController.get(), &mccuav::UavController::uavFirmwareLoaded, this, &MccPlotWidget::loadSettings);
    connect(_uavController.get(), &mccuav::UavController::uavReadyForExchange, this, &MccPlotWidget::deviceActivated);
}

MccPlotWidget::~MccPlotWidget()
{
    delete _proxyModel;
}

void MccPlotWidget::tmParamList(const mccmsg::tm::ParamList& paramsList)
{
    for (auto p : paramsList.params())
    {
        QString name = QString("%1.%2.%3").arg(paramsList.device().qstringify())
            .arg(QString::fromStdString(p.trait()))
            .arg(QString::fromStdString(p.status()));

        if (!_listenParams.contains(name))
            continue;

        quint64 msecsToNow = bmcl::toMsecs(paramsList.message_time().time_since_epoch()).count();

        _ui.plot->curveValueChanged(name, (double)msecsToNow, p.value().toDouble());
    }
}

void MccPlotWidget::timeIntervalChanged(int index)
{
    int interval = _ui.timeInterval->itemData(index).toInt();
    _ui.plot->setInterval((double)interval);
}

void MccPlotWidget::tmParamSubscribeChanged(TmParamTreeItem* item, bool subscribe)
{
    QString paramName = QString("%1.%2.%3").arg(item->device().qstringify()).arg(item->trait()).arg(item->name());

    QString trait = item->shortTrait();

    auto dev = _uavController->uav(item->device());
    if (dev.isNone())
        return;

    if (subscribe && !_listenParams.contains(paramName))
    {
        _listenParams.append(paramName);

        QString infoName = QString("%1/%2/%3")
            .arg(QString::fromStdString(dev.unwrap()->deviceDescription()._device_info))
            .arg(item->parent()->info()).arg(item->info());

        _ui.plot->addCurve(paramName, infoName, findFreeColor(), item);

        if (dev->statDevice()._isActive)
        {
            _uavController->startReadingParam(item->device().qstringify(), item->trait(), (int)item->number(), 1);
        }
        else
        {
            _startReadQueue.append(item);
        }
    }
    else if (!subscribe && _listenParams.contains(paramName))
    {
        _listenParams.removeAt(_listenParams.indexOf(paramName));
        _ui.plot->removeCurve(paramName);
        if (dev->statDevice()._isActive)
        {
            _uavController->stopReadingParam(item->device().qstringify(), item->trait(), (int)item->number(), 1);
        }
        else
        {
            _stopReadQueue.append(item);
        }
    }

    if (!_isLoadingSettings)
        saveSettings();
}

void MccPlotWidget::loadSettings(mccuav::Uav* dev)
{
    _isLoadingSettings = true;
    auto plotList = mccui::Settings::instance()->plotSettingsList();

    for (auto curveSettings : plotList)
    {
        if (curveSettings.trait.device != dev->id().qstringify())
            continue;

        _proxyModel->subscribeTmParam(curveSettings.trait.device, curveSettings.trait.trait, curveSettings.trait.name);
    }

    _isLoadingSettings = false;
}

void MccPlotWidget::saveSettings()
{
    mccui::PlotSettingsList plotList;
    for (auto curve : _ui.plot->curves())
    {
        mccui::PlotSettings curveSettings;
        curveSettings.trait.device = curve->variable()->device().qstringify();
        curveSettings.trait.trait = curve->variable()->trait();
        curveSettings.trait.name = curve->variable()->name();
        curveSettings.color = curve->color();
        curveSettings.multiplier = curve->multiplier();
        curveSettings.offset = curve->offset();
        plotList.append(curveSettings);
    }

    mccui::Settings::instance()->setPlotSettingsList(plotList);
}

void MccPlotWidget::deviceActivated(mccuav::Uav* dev)
{
    QVector<TmParamTreeItem*> toRemove;

    for (auto p : _startReadQueue)
    {
        if (p->device() == dev->id())
        {
            _uavController->startReadingParam(p->device().qstringify(), p->trait(), (int)p->number(), 1);
            toRemove.append(p);
        }
    }

    for (auto r : toRemove)
    {
        _startReadQueue.remove(_startReadQueue.indexOf(r));
    }

    toRemove.clear();

    for (auto p : _stopReadQueue)
    {
        if (p->device() == dev->id())
        {
            _uavController->stopReadingParam(p->device().qstringify(), p->trait(), (int)p->number(), 1);
            toRemove.append(p);
        }
    }

    for (auto r : toRemove)
    {
        _stopReadQueue.remove(_stopReadQueue.indexOf(r));
    }
}

QColor MccPlotWidget::findFreeColor() const
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
        for (auto it : _ui.plot->curves())
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
    return colors[_ui.plot->curves().count() % numColors];
}

}
