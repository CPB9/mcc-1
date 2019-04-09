#include "AirframeWidget.h"
#include "AirframesModel.h"
#include "AirframeViewDelegate.h"

#include <bmcl/Logging.h>


#include <QFile>
#include <QVBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QListView>

#include <QAbstractListModel>
#include <QStyledItemDelegate>

#include <QApplication>
#include <QStyle>
#include <QDomDocument>
#include <QPushButton>

#include <QImage>
#include <QPainter>

#include "mcc/msg/ParamList.h"
#include "mcc/msg/SubHolder.h"
#include "mcc/ui/FlowLayout.h"

#include "mcc/uav/UavController.h"
#include "mcc/uav/ChannelsController.h"

namespace mccmav {

AirframeWidget::~AirframeWidget()
{
}

AirframeWidget::AirframeWidget(const mccmsg::Protocol& protocol, mccuav::UavController* uavController, QWidget* parent)
    : _protocol(protocol)
    , _uavController(uavController)
    , _uav(nullptr)
{
    auto layout = new QVBoxLayout();
    setLayout(layout);

    _currentAirframeLabel = new QLabel(this);
    layout->addWidget(_currentAirframeLabel);

    _airframesView = new QListView(parent);
    layout->addWidget(_airframesView);

    _model = new AirframesModel(this);
    _airframesView->setModel(_model);
    _airframesView->setViewMode(QListView::ViewMode::IconMode);

    auto delegate = new AirframeViewDelegate(this);
    _airframesView->setItemDelegate(delegate);

    connect(delegate, &AirframeViewDelegate::forceSelection, this,
            [this](const QModelIndex& index)
            {
                _airframesView->selectionModel()->select(index, QItemSelectionModel::SelectionFlag::ClearAndSelect);
            }
    );

    connect(_airframesView->selectionModel(), &QItemSelectionModel::currentChanged, this,
            [this, delegate](const QModelIndex &current, const QModelIndex &previous)
            {
                emit delegate->updateSelection(current);
            }
    );
    QPushButton* applyAndRestart = new QPushButton("Apply and Restart", this);
    applyAndRestart->setVisible(false);
    _rebootWarning = new QLabel(this);
    layout->addWidget(_rebootWarning);
    _rebootWarning->setVisible(false);
    _rebootWarning->setStyleSheet("QLabel { color : red; }");

    layout->addWidget(applyAndRestart);

    connect(delegate, &AirframeViewDelegate::airframeSelected, this,
            [this, applyAndRestart](int airframeIndex)
            {
                _airframeIndex = airframeIndex;
                bool isSame = (_model->currentAirframe() == airframeIndex);
                _rebootWarning->setText(QString("Airframe will be changed from %1 to %2. All system parameters will be reset to defaults, device will be rebooted")
                                        .arg(_model->currentAirframe()).arg(airframeIndex));
                applyAndRestart->setVisible(!isSame);
                _rebootWarning->setVisible(!isSame);
            }
    );


    connect(applyAndRestart, &QPushButton::clicked, this,
            [this]()
            {
                if (_airframeIndex.isNone())
                {
                    assert(false);
                    return;
                }
                _uavController->sendCmdYY(new mccmsg::CmdParamList(_uav->device(), "System", "setAirframe", { _airframeIndex.unwrap() }));
            }
    );


    loadXmlModel();
    using mccuav::UavController;

    auto manager = _uavController.get();
    connect(manager, &UavController::selectionChanged, this, &AirframeWidget::selectionChanged);
    connect(manager, &UavController::uavFirmwareLoaded, this, &AirframeWidget::selectionChanged);

    selectionChanged(manager->selectedUav());
}

void AirframeWidget::selectionChanged(mccuav::Uav* uav)
{
    if (!uav)
    {
        _uav = nullptr;
        _airframeIndex = bmcl::None;
        return;
    }

    if (uav != _uavController->selectedUav())
        return;

    if (uav->protocol() != _protocol)
        return;
    _uav = uav;
    if (!_uav)
        return;

    if (uav->tmStorage().isNull())
        return;

    _tmStorage = bmcl::dynamic_pointer_cast<TmStorage>(uav->tmStorage());
    if (_tmStorage.isNull())
        return;

    auto updater = [this](const ParamValue& p)
    {
        if (p.name != "SYS_AUTOSTART")
            return;

        setAirframeId(p.value.toInt());
    };

    _tmStorage->namedAccess().addHandler(std::move(updater)).takeId();
    auto af = _tmStorage->valueByName("SYS_AUTOSTART");
    if (af.isSome())
    {
        setAirframeId(af->value.toInt());
    }
}

// void AirframeWidget::tmParamList(const mccmsg::TmParamListPtr& params)
// {
//     if (!_uav)
//         return;
// 
//     if (params->device() != _uav->device())
//         return;
// 
//     auto sysAutostartParam = params->findParam("", "SYS_AUTOSTART");
//     if (sysAutostartParam.isSome())
//     {
//         int sysAutoStart = sysAutostartParam->value().asInt();
//         _model->setCurrentAirframe(sysAutoStart);
//         _currentAirframeLabel->setText(QString("Current airframe: %1").arg(sysAutoStart));
//     }
// }

void AirframeWidget::loadXmlModel()
{
    QFile file(":/resources/AirframeFactMetaData.xml");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        BMCL_WARNING() << "Can not open AirframeFactMetaData.xml";
        return;
    }

    QDomDocument doc;
    if (!doc.setContent(&file))
    {
        BMCL_WARNING() << "Can not read AirframeFactMetaData.xml";
        return;
    }

    QDomNodeList xmlRoot = doc.elementsByTagName("airframe_group");
    for (int i = 0; i < xmlRoot.size(); i++)
    {
        QDomNode tmp = xmlRoot.item(i);
        if (!tmp.isElement())
            continue;

        QDomElement xmlAirframeGroup = tmp.toElement();
        QString groupName = xmlAirframeGroup.attribute("name");
        QString image = xmlAirframeGroup.attribute("image");

        QMap<int, QString> airframes;
        QDomNodeList xmlAirframe = xmlAirframeGroup.elementsByTagName("airframe");
        //int q = xmlAirframe.size();
        for (auto j = 0; j < xmlAirframe.size(); ++j)
        {
            QDomElement t = xmlAirframe.item(j).toElement();
            int id = t.attribute("id", "-1").toInt();
            QString name = t.attribute("name", "<noname>");
            airframes[id] = name;
        }

        _model->addAiframeGroup(groupName, image, airframes);
     }
    file.close();

    for (int i = 0; i < _model->rowCount(); ++i)
    {
        _airframesView->openPersistentEditor(_model->index(i));
    }
}

void AirframeWidget::setAirframeId(int af)
{
    _model->setCurrentAirframe(af);
    _currentAirframeLabel->setText(QString("Current airframe: %1").arg(af));
}

}
