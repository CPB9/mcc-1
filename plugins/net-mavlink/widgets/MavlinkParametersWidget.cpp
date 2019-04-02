#include "../widgets/MavlinkParametersWidget.h"
#include "../widgets/FirmwareModel.h"
#include "../widgets/ParameterEditor.h"

#include "mcc/msg/ParamList.h"

#include "mcc/uav/UavController.h"
#include "mcc/uav/ChannelsController.h"
#include "mcc/uav/Uav.h"

#include <QTableView>
#include <QListView>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSplitter>
#include <QHeaderView>
#include <QItemDelegate>

#include <QStringListModel>
#include <QSortFilterProxyModel>

namespace mccmav {

class DrawWithoutFocusDelegate : public QItemDelegate
{
public:
    virtual void drawFocus(QPainter * /*painter*/, const QStyleOptionViewItem & /*option*/, const QRect & /*rect*/) const {}
};

class CategoryFilterModel : public QSortFilterProxyModel
{
public:
    explicit CategoryFilterModel(QObject* parent)
        : QSortFilterProxyModel(parent)
    {

    }

    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
    {
        QModelIndex index0 = sourceModel()->index(sourceRow, 0, sourceParent);
        const auto& desc = index0.data(Qt::UserRole).value<ParameterDescription>();
        QString cat = QString::fromStdString(desc.category);
        return cat.contains(filterRegExp());
    }
};

MavlinkParametersWidget::MavlinkParametersWidget(mccuav::ChannelsController* chanController,
                                                 mccuav::UavController* uavController,
                                                 QWidget* parent)
    : QWidget(parent)
    , _uav(nullptr)
    , _parameterEditor(nullptr)
    , _chanController(chanController)
    , _uavController(uavController)
{
    using mccuav::UavController;

    _model = new FirmwareModel(this);

    _sortModel = new CategoryFilterModel(this);
    _sortModel->setSourceModel(_model);

    _categoriesModel = new QStringListModel(this);

    setStyleSheet("background-color: #222222");
    _view = new QTableView();
    _drawWithoutFocusDelegate = new DrawWithoutFocusDelegate();
    _view->setItemDelegate(_drawWithoutFocusDelegate);

    _view->setStyleSheet("QTableView {\
    background-color: #222222; \
    gridline-color: white\
}\
QTableView::item\
{\
    border-style: none;\
    border-bottom: 1px solid #434343;\
}");
    _view->setShowGrid(false);
    _view->setModel(_sortModel);
    _view->horizontalHeader()->setStretchLastSection(true);
    _view->verticalHeader()->setVisible(false);
    _view->horizontalHeader()->setVisible(false);
    _view->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
    _view->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);

    _filter = new QLineEdit();
    _categories = new QListView();
    _categories->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _categories->setModel(_categoriesModel);

    _categories->setStyleSheet("QListView { \
    background-color: #222222;              \
}                                           \
                                            \
QListView::item {                           \
    background-color: #626264;              \
    margin: 2px 2px 2px 2px;                \
    padding: 5px 1px 5px 1px;               \
    color: rgb(255, 255, 255);              \
}                                           \
                                            \
QListView::item:hover {                     \
    background-color: #edeb33;              \
    color: black;                           \
}                                           \
                                            \
QListView::item:selected{           \
    background-color: #edeb33;              \
    color: black;                           \
    border:none;                            \
}                                           \
");

    _categories->setFocusPolicy(Qt::NoFocus);

    auto selectionModel = _categories->selectionModel();
    connect(selectionModel, &QItemSelectionModel::currentRowChanged, this,
            [this](const QModelIndex &current, const QModelIndex &previous)
            {
                QString text = current.data().toString();
                QRegExp regExp = QRegExp(text, Qt::CaseInsensitive, QRegExp::RegExp);
                _sortModel->setFilterRegExp(regExp);
                _view->resizeColumnToContents(0);

                std::string trait;
                std::vector<std::string> vars;
                for (int i = 0; i < _sortModel->rowCount(); ++i)
                {
                    auto var = _sortModel->data(_sortModel->index(i, 0), Qt::DisplayRole).toString().toStdString();
                    ParameterDescription d = _sortModel->data(_sortModel->index(i, 0), Qt::UserRole).value<ParameterDescription>();
                    vars.emplace_back(var);
                }
                _uavController->sendCmdYY(new mccmsg::CmdParamRead(_uav->device(), trait, vars));
            }
    );

//    connect(_filter, &QLineEdit::textChanged, this,
//            [this](const QString& text)
//            {
//                QRegExp regExp = QRegExp(text, Qt::CaseInsensitive, QRegExp::RegExp);
//                _filterModel->setFilterRegExp(regExp);
//            }
//    );

    auto leftPanelLayout = new QVBoxLayout();
    leftPanelLayout->addWidget(_filter);
    leftPanelLayout->addWidget(_categories, 1);

    QSplitter* mainSplitter = new QSplitter(Qt::Horizontal);
    mainSplitter->setHandleWidth(1);
    mainSplitter->setContentsMargins(3, 3, 3, 3);

    auto leftPanel = new QWidget();
    leftPanel->setLayout(leftPanelLayout);

    mainSplitter->addWidget(leftPanel);
    mainSplitter->addWidget(_view);

    auto mainLayout = new QHBoxLayout();
    mainLayout->addWidget(mainSplitter);

    setLayout(mainLayout);

    auto manager = _uavController.get();
    //connect(manager, &UavController::tmParamList, this, &MavlinkParametersWidget::tmParamList);

    connect(_view, &QTableView::doubleClicked, this,
            [this](QModelIndex index)
            {
                ParameterDescription d = index.data(Qt::UserRole).value<ParameterDescription>();
                QVariant paramValue = index.data(Qt::UserRole + 1);
                if (paramValue.isNull())
                    return;

                delete _parameterEditor;
                _parameterEditor = new ParameterEditor(this, d, mccmsg::NetVariant(paramValue));

                connect(_parameterEditor, &ParameterEditor::accepted, this,
                        [this, d]()
                {
                    std::vector<std::pair<std::string, mccmsg::NetVariant>> vars;
                    vars.emplace_back(d.name, _parameterEditor->value());
                    _uavController->sendCmdYY(new mccmsg::CmdParamWrite(_uav->device(), "", vars));
                });

                _parameterEditor->show();
            }
    );
}

MavlinkParametersWidget::~MavlinkParametersWidget()
{
    delete _drawWithoutFocusDelegate;
}

void MavlinkParametersWidget::selectionChanged(mccuav::Uav* device)
{
    if (!device)
    {
        _uav = nullptr;
        return;
    }

    _uav = device;
    if (!_uav)
        return;

    if (device->tmStorage().isNull())
        return;

    _model->setTmStorage(device->tmStorage());

    QStringList cats;
    for (const auto& it : _model->firmware()->paramsDescription())
    {
        QString name = QString::fromStdString(it.category);
        if (!cats.contains(name) && !name.isEmpty())
            cats << name;
    }
    cats.sort();

    _categoriesModel->setStringList(cats);
    _categories->setCurrentIndex(_categoriesModel->index(0, 0));
}

void MavlinkParametersWidget::firmwareLoaded(mccuav::Uav* uav)
{
    if (_uav == uav)
    {
        selectionChanged(uav);

//         if (!uav->fsmStates().hasParser() && _uav->firmwareDescription().isSome())
//         {
//             auto fw = _uav->firmwareDescription()->frm();
//             auto mavFw = bmcl::dynamic_pointer_cast<const Firmware>(fw);
//             if (mavFw.isNull())
//                 return;
// 
//             auto fsmFunc = [mavFw, uav](uint16_t state1, uint16_t state2) -> std::pair<std::string, bmcl::Option<std::string>>
//             {
//                 std::string s1;
//                 switch (state1)
//                 {
//                     case 0: s1 = "Manual"; break;
//                     case 1: s1 = "Stabilized"; break;
//                     case 2: s1 = "Acro"; break;
//                     case 3: s1 = "RAttitude"; break;
//                     case 4: s1 = "AltCtl"; break;
//                     case 5: s1 = "PosCtl"; break;
//                     case 6: s1 = "Hold"; break;
//                     case 7: s1 = "Mission"; break;
//                     case 8: s1 = "RTL"; break;
//                     case 9: s1 = "Folow"; break;
//                     case 10: s1 = "Auto.Land"; break;
//                     case 11: s1 = "Auto.Ready"; break;
//                     case 12: s1 = "Auto.Rtgs"; break;
//                     case 13: s1 = "Auto.Takeoff"; break;
//                     default:
//                         s1 = "Unknown??";
//                 }
//                 bmcl::Option<std::string> s2 = (state2 == 1 ? std::string("Armed"): std::string("Disarmed"));
//                 return std::make_pair(s1, s2);
//             };
//             uav->fsmStates().setParser(std::move(fsmFunc));
//         }
    }
}

// void MavlinkParametersWidget::tmParamList(const mccmsg::TmParamListPtr& params)
// {
//     if (!_uav)
//         return;
// 
//     if (_uav->device() != params->device())
//         return;
// 
//     for (const auto& p : params->params())
//     {
//         _model->setParam(p);
//     }
//}
}
