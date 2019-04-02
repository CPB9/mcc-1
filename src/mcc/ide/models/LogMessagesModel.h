#pragma once
#include <memory>
#include <deque>

#include <QAbstractTableModel>
#include <QColor>

#include "mcc/Config.h"

#include <bmcl/Logging.h>
#include <bmcl/Option.h>

#include "mcc/msg/ptr/Fwd.h"
#include "mcc/msg/Fwd.h"
#include "mcc/uav/Fwd.h"

namespace mccide {

class AbstractLogMessage
{
public:
    AbstractLogMessage(bmcl::LogLevel logLevel);
    virtual ~AbstractLogMessage();

    virtual const QString& time() const;
    virtual const QString& component() const;
    virtual const QString& deviceName() const;
    virtual const QString& message() const;
    virtual const QColor&  messageColor() const;
    virtual bool           canCancel() const;
    virtual mccmsg::RequestId cmdId() const;

    const QColor& deviceColor() const { return _deviceColor; }
    const bmcl::Option<mccmsg::Device>& device() const { return _device; }
    bmcl::LogLevel         logLevel() const { return _logLevel; }
protected:
    QString                         _time;
    QString                         _component;
    bmcl::Option<mccmsg::Device>    _device;
    QString                         _deviceName;
    QString                         _message;
    QColor                          _deviceColor;
    QColor                          _messageColor;
    bmcl::LogLevel                  _logLevel;
};

typedef std::unique_ptr<AbstractLogMessage> LogMessagePtr;

class MCC_IDE_DECLSPEC LogMessagesModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    enum Roles
    {
        CanCancel = Qt::UserRole + 1,
        CmdId,
        LogLevel,
        DeviceRole,
        TimeRole,
        ComponentRole,
        DeviceNameRole,
        MessageRole,
        DeviceColorRole,
        MessageColorRole,
    };

    LogMessagesModel(QObject* parent);
    ~LogMessagesModel() override;

    void setContext(mccuav::UavController* context);

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    using QObject::parent;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    void timerEvent(QTimerEvent*) override;

signals:
    void onLogMessage(const LogMessagePtr& logMessage);

private slots:
    void onLog(const mccmsg::tm::LogPtr& logMessage);
    void onCmdResponse(const mccmsg::DevReqPtr& req, const bmcl::Result<mccmsg::CmdRespAnyPtr, mccmsg::ErrorDscr>& rep);

private:
    QModelIndex parent(const QModelIndex &child) const override;

    void addMessage(LogMessagePtr&& msgPtr);
    mccuav::UavController* _context;
    std::vector<LogMessagePtr> _messages;
    std::deque<LogMessagePtr> _queue;
};
}

Q_DECLARE_METATYPE(mccmsg::Device);
