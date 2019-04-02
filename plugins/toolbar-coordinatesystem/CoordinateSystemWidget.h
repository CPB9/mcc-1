#pragma once

#include "mcc/Config.h"
#include "mcc/uav/Fwd.h"
#include "mcc/ui/Fwd.h"
#include "mcc/ui/Rc.h"

#include <QWidget>

class QLabel;

class CoordinateSystemPropertiesDialog;

class CoordinateSystemWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CoordinateSystemWidget(mccui::CoordinateSystemController* csController,
                                    mccuav::GlobalActions* actions,
                                    QWidget *parent = nullptr);
    ~CoordinateSystemWidget() override;

    bool eventFilter(QObject *watched, QEvent *event) override;

public slots:
    void updateSystemAndFormat();

public:
    bool event(QEvent *event) override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    mccui::Rc<mccui::CoordinateSystemController> _csController;

    QLabel*                             _system;
    QLabel*                             _format;

    CoordinateSystemPropertiesDialog*   _propertiesDialog;
    bool                                _hovered;

    Q_DISABLE_COPY(CoordinateSystemWidget)
};
