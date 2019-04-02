#pragma once
#include "mcc/uav/Rc.h"
#include "mcc/uav/Fwd.h"
#include <QWidget>

class QSortFilterProxyModel;
class QPoint;
class QMenu;
class QSortFilterProxyModel;
class QTreeView;
class QLineEdit;
class QSortFilterProxyModel;
class QCheckBox;
class QComboBox;

namespace mccide {
class LogMessagesModel;
}
class MySortModel;

class MessageListTool : public QWidget
{
    Q_OBJECT
public:
    MessageListTool(mccuav::UavController* uavController, QWidget* parent = nullptr);
    ~MessageListTool();

    mccide::LogMessagesModel* logModel() const;
public slots:
    void scrollDown();
private:
    mccuav::Rc<mccuav::UavController>   _uavController;
    QCheckBox*                          _autoScrollCheck;
    QComboBox*                          _logLevelCombo;
    QTreeView*                          _messagesTree;
    QLineEdit*                          _filterLine;
    mccide::LogMessagesModel*           _logModel;
    MySortModel*                        _proxyModel;
};
