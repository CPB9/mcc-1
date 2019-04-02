#pragma once

#include "mcc/msg/Objects.h"
#include "mcc/uav/Fwd.h"
#include "mcc/uav/Rc.h"
#include "mcc/ui/Dialog.h"
#include "mcc/ui/Fwd.h"

class QLineEdit;
class QLabel;
class QCheckBox;

class EditUavDialog : public mccui::Dialog
{
    Q_OBJECT
public:
    explicit EditUavDialog(mccuav::UavController* uavController,
                           QWidget *parent = nullptr);
    ~EditUavDialog() override;

    void setUav(const mccmsg::Device& deviceId);
    const mccmsg::Device& uav() const;

    void setName(const QString& text);
    QString name() const;

    bool eventFilter(QObject *watched, QEvent *event) override;

public slots:
    void accept() override;

private:
    mccuav::Rc<mccuav::UavController>   _uavController;
    mccmsg::Device                      _deviceId;

    QLineEdit*                          _name;
    QLabel*                             _colorSelector;
    QColor                              _uavColor;
    QCheckBox*                          _uavLogging;
    Q_DISABLE_COPY(EditUavDialog)
};
