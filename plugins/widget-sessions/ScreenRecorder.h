#pragma once

#include <QObject>

#include "mcc/ui/Rc.h"
#include "mcc/ui/Fwd.h"
#include <QTime>
#include <QStorageInfo>
#include <QProcess>
#include <QRect>

class ScreenRecorder : public QObject
{
    Q_OBJECT
public:
    enum class State
    {
        Started,
        Paused,
        Stopped
    };

    explicit ScreenRecorder(mccui::Settings* settings, QObject* parent = nullptr);
    ~ScreenRecorder();
    const QTime& recordTime() const;
    const QString& currentSettings() const;
    uint64_t bytesAvailable() const;
    bool isPaused() const;
    State state() const;

    void setFileName(const QString& fileName);

    static QString defaultFfmpegPath();
    static int defaultFps();
    static QString defaultCodec();
    static std::vector<QString> availableCodecs();

    static QString genCommandLine(const QString& path, size_t framerate, const QString& codec);
public slots:
    void startRecord();
    void stopRecord();
    void pauseRecord();
signals:
    void started();
    void paused();
    void stopped();
    void recordTimeChanged();
    void storageInfoChanged();
    void error(const QString& text);
    void message(const QString& text);
protected:
    virtual void timerEvent(QTimerEvent *event) override;

private:
    void setRecordTime(const QTime& t);
    void updateStorageInfo();
    void updatePropertiesInfo();
    void updateTimeInfo();

private:
    State _state;
    QString      _sessionFileName;
    QString      _currentSettings;
    QString      _recordsPath;
    QStorageInfo _storageInfo;
    QProcess     _videoProcessor;
    QTime        _recordTime;
    QRect        _recordRect;
    bool  _isRecordPaused;
    QString _ffmpegPath;
    bmcl::Rc<mccui::SettingsReader>  _ffmpegPathReader;
    bmcl::Rc<mccui::SettingsReader>  _fpsReader;
    bmcl::Rc<mccui::SettingsReader>  _codecReader;
};



