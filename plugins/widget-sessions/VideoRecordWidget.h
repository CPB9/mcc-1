#pragma once

#include <QWidget>
#include "mcc/ui/Rc.h"
#include "mcc/ui/Fwd.h"
#include "mcc/ui/Settings.h"
#include "mcc/uav/Fwd.h"

class ScreenRecorder;
class QLabel;

class VideoRecordWidget : public QWidget
{
    Q_OBJECT
public:
    VideoRecordWidget(const mccui::Rc<mccuav::UavController>& uavController, const mccui::Rc<mccui::Settings>& settings, QWidget* parent);

    void setFileName(const QString& fileName);
    void startRecord();
    void stopRecord();
private slots:
    void updateData();
private:
    mccui::OnOffSliderCheckBox* _slider;
    ScreenRecorder* _recorder;
    QLabel* _recordTime;
    QLabel* _recordRect;
    QLabel* _availableSize;
    bmcl::Rc<mccui::SettingsWriter> _recordEnabledWriter;
};