#include "MessageListTool.h"

#include "mcc/ide/models/LogMessagesModel.h"
#include "mcc/msg/ptr/Tm.h"
#include "mcc/uav/UavController.h"
#include "mcc/uav/UavController.h"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDateTime>
#include <QDebug>
#include <QHeaderView>
#include <QLineEdit>
#include <QMenu>
#include <QMenu>
#include <QSortFilterProxyModel>
#include <QStyle>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QWidget>
#include <QtGlobal>

class MySortModel : public QSortFilterProxyModel
{
public:
    explicit MySortModel(QObject *parent = 0)
        : QSortFilterProxyModel(parent)
    {
        setLogLevel(bmcl::LogLevel::Debug);
    }

    void setLogLevel(bmcl::LogLevel logLevel)
    {
        beginResetModel();
        _logLevel = logLevel;
        endResetModel();
    }
protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
    {
        QModelIndex index0 = sourceModel()->index(sourceRow, 0, sourceParent);
        QModelIndex index1 = sourceModel()->index(sourceRow, 1, sourceParent);
        QModelIndex index2 = sourceModel()->index(sourceRow, 2, sourceParent);
        QModelIndex index3 = sourceModel()->index(sourceRow, 3, sourceParent);

        int logLevel = sourceModel()->data(index0, mccide::LogMessagesModel::LogLevel).toInt();
        if (logLevel > (int)_logLevel)
            return false;

        return (sourceModel()->data(index0).toString().contains(filterRegExp())
            || sourceModel()->data(index1).toString().contains(filterRegExp())
            || sourceModel()->data(index2).toString().contains(filterRegExp())
            || sourceModel()->data(index3).toString().contains(filterRegExp()));
    }
private:
    bmcl::LogLevel _logLevel;
};

MessageListTool::MessageListTool(mccuav::UavController* uavController, QWidget* parent)
    : QWidget(parent)
    , _uavController(uavController)
{
    setObjectName("Телеметрические сообщения");
    setWindowTitle("Телеметрические сообщения");
    setWindowIcon(QIcon(":/logmessages/icon.png"));

    _logModel = new mccide::LogMessagesModel(this);
    _logModel->setContext(uavController);

    _messagesTree = new QTreeView(this);
    _messagesTree->setContextMenuPolicy(Qt::CustomContextMenu);
    _messagesTree->setAlternatingRowColors(true);

    _filterLine = new QLineEdit(this);
    _filterLine->setPlaceholderText("Введите строку поиска...");

    _proxyModel = new MySortModel(this);
    _proxyModel->setSourceModel(_logModel);

    _messagesTree->setModel(_proxyModel);

    _autoScrollCheck = new QCheckBox("Автопрокрутка", this);
    _autoScrollCheck->setChecked(true);
    _logLevelCombo = new QComboBox(this);
    _logLevelCombo->addItem("Debug", (int)bmcl::LogLevel::Debug);
    _logLevelCombo->addItem("Info", (int)bmcl::LogLevel::Info);
    _logLevelCombo->addItem("Warning", (int)bmcl::LogLevel::Warning);
    _logLevelCombo->addItem("Critical", (int)bmcl::LogLevel::Critical);

    QVBoxLayout* layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    QHBoxLayout* buttonsLayout = new QHBoxLayout();
    buttonsLayout->addWidget(_logLevelCombo);
    buttonsLayout->addWidget(_filterLine);
    buttonsLayout->addWidget(_autoScrollCheck);
    layout->addLayout(buttonsLayout);
    layout->addWidget(_messagesTree);
    setLayout(layout);

    connect(_logModel, &QAbstractTableModel::rowsInserted, this, &MessageListTool::scrollDown);

    connect(_filterLine, &QLineEdit::textChanged, this,
        [this](const QString& text)
        {
            QRegExp regExp = QRegExp(text, Qt::CaseInsensitive, QRegExp::RegExp);
            _proxyModel->setFilterRegExp(regExp);
        }
    );

    connect(_logLevelCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            [this](int index)
            {
                int level = _logLevelCombo->currentData().toInt();
                _proxyModel->setLogLevel((bmcl::LogLevel)level);
            }
    );
    setTabOrder(_filterLine, _messagesTree);
}

MessageListTool::~MessageListTool()
{
}


mccide::LogMessagesModel* MessageListTool::logModel() const
{
    return _logModel;
}

void MessageListTool::scrollDown()
{
    if(_autoScrollCheck->isChecked())
        _messagesTree->scrollToBottom();
}
