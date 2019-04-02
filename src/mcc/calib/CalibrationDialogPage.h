#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>

#include "mcc/calib/CalibrationControllerAbstract.h"

namespace mcccalib {

class CalibrationDialogPage : public QWidget
{
    Q_OBJECT

signals:
    void statusChanged(bool status);
    void started(bool started);

public:
    CalibrationDialogPage(CalibrationControllerAbstract* widget)
        : _widget(widget)
    {
        QVBoxLayout* layout = new QVBoxLayout();
        QPushButton* startButton = new QPushButton("Начать калибровку");
        QPushButton* cancelButton = new QPushButton("Отменить");

        layout->addWidget(startButton);
        layout->addWidget(widget);
        layout->addWidget(cancelButton);

        connect(startButton, &QPushButton::pressed, this,
                [this, startButton, widget, cancelButton]()
                {
                    startButton->setVisible(false);
                    widget->setVisible(true);
                    cancelButton->setVisible(true);
                    widget->start();
                    emit started(true);
                }
        );

        connect(cancelButton, &QPushButton::pressed, this,
                [this, startButton, widget, cancelButton]()
                {
                    startButton->setVisible(true);
                    widget->setVisible(true);
                    cancelButton->setVisible(false);
                    widget->cancel();
                    emit started(false);
                }
        );

        setLayout(layout);

        startButton->setVisible(true);
        cancelButton->setVisible(false);
        widget->setVisible(true);

        connect(_widget, &CalibrationControllerAbstract::cancelled, this, [cancelButton, startButton]() { cancelButton->setVisible(false); startButton->setVisible(true); });
        connect(_widget, &CalibrationControllerAbstract::completed, this, [cancelButton, startButton]() { cancelButton->setVisible(false); startButton->setVisible(true); });
        connect(_widget, &CalibrationControllerAbstract::failed,    this, [cancelButton, startButton]() { cancelButton->setVisible(false); startButton->setVisible(true); });
    }

    void setDevice(const mccmsg::Device& device)
    {
        _widget->setDevice(device);
    }

    mccmsg::CalibrationSensor sensor() const { return _widget->sensor(); }
    CalibrationControllerAbstract* controller() { return _widget; };
private:
    QString _info;
    CalibrationControllerAbstract* _widget;
};
}
