#include "ScreenRecorder.h"

#include "mcc/ui/Settings.h"
#include "mcc/ui/TextUtils.h"
#include "mcc/path/Paths.h"

#include <bmcl/Logging.h>

#if defined(BMCL_PLATFORM_LINUX)
#include <signal.h>
#elif defined(BMCL_PLATFORM_WINDOWS)
#endif

#include <QApplication>
#include <QDateTime>
#include <QDesktopWidget>
#include <QFont>
#include <QProcess>
#include <QStorageInfo>
#include <QTimer>
#include <QDebug>

ScreenRecorder::ScreenRecorder(mccui::Settings* settings, QObject* parent /*= nullptr*/)
    : QObject(parent)
    , _recordsPath(mccpath::qGetLogsPath())
    , _isRecordPaused(false)
    , _state(ScreenRecorder::State::Stopped)
    , _recordTime(QTime(0, 0, 0))
{
    _ffmpegPathReader = settings->acquireReader("videorecorder/ffmpegPath", ScreenRecorder::defaultFfmpegPath());
    _fpsReader = settings->acquireReader("videorecorder/fps", ScreenRecorder::defaultFps());
    _codecReader = settings->acquireReader("videorecorder/codec", ScreenRecorder::defaultCodec());

    _storageInfo.setPath(_recordsPath);
    connect(&_videoProcessor, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this,
            [this](int exitCode, QProcess::ExitStatus exitStatus)
            {
                _isRecordPaused = false;
                _state = State::Stopped;
                emit stopped();

                BMCL_DEBUG() << "ScreenRecorderWidget. Process \""
                    << _videoProcessor.program()
                    << "\" finished, exit code "
                    << exitCode
                    << ". Status: " << exitStatus;
                if(exitCode != 0)
                {
                    auto err = _videoProcessor.readAll();
                    QString xx = QString::fromLocal8Bit(err);
                    emit error(QString("Процесс записи видео остановлен: %1").arg(xx));
                }
                emit message(QString("Остановлена запись видео."));
                setRecordTime(QTime(0,0,0));
            });

    connect(&_videoProcessor, &QProcess::started, this,
            [this]()
            {
                emit message(QString("Запущена запись видео."));
            });

#if QT_VERSION >= 0x050600
    connect(&_videoProcessor, &QProcess::errorOccurred, this,
            [this](QProcess::ProcessError e)
            {
                _isRecordPaused = false;
                _state = State::Stopped;
                emit stopped();
                setRecordTime(QTime(0, 0, 0));
                QString err;
                switch(e)
                {
                case QProcess::FailedToStart:
                    err = "Failed to Start " + _ffmpegPathReader->read().toString();
                    break;
                case QProcess::Crashed:
                    err = "Crashed";
                    break;
                case QProcess::Timedout:
                    err = "Timedout";
                    break;
                case QProcess::WriteError:
                    err = "WriteError";
                    break;
                case QProcess::ReadError:
                    err = "ReadError";
                    break;
                case QProcess::UnknownError:
                    err = "UnknownError";
                    break;
                }
                emit error(QString("Ошибка записи видео: %1").arg(err));
            });
#endif

    startTimer(1000);
    updatePropertiesInfo();
}

ScreenRecorder::~ScreenRecorder()
{
    stopRecord();
}

const QTime& ScreenRecorder::recordTime() const
{
    return _recordTime;
}

const QString& ScreenRecorder::currentSettings() const
{
    return _currentSettings;
}

uint64_t ScreenRecorder::bytesAvailable() const
{
    return _storageInfo.bytesAvailable();
}

bool ScreenRecorder::isPaused() const
{
    return _isRecordPaused;
}

ScreenRecorder::State ScreenRecorder::state() const
{
    return _state;
}

void ScreenRecorder::setFileName(const QString& fileName)
{
    _sessionFileName = fileName;
}

QString ScreenRecorder::defaultFfmpegPath()
{
#if defined(BMCL_PLATFORM_WINDOWS)
    return mccpath::qGetBinPath() + "/ffmpeg/bin/ffmpeg";
#else
    return "/usr/bin/ffmpeg";
#endif
}

int ScreenRecorder::defaultFps()
{
    return 15;
}

QString ScreenRecorder::defaultCodec()
{
    return ScreenRecorder::availableCodecs()[0];
}

std::vector<QString> ScreenRecorder::availableCodecs()
{
    return { "libx264", "h264_nvenc" };
}

QString ScreenRecorder::genCommandLine(const QString& path, size_t framerate, const QString& codec)
{
    return QString("%1 -f gdigrab -framerate %2 -i desktop -c:v %3").arg(path).arg(framerate).arg(codec);
}

void ScreenRecorder::startRecord()
{
    if(_recordsPath.isNull())
    {
        BMCL_WARNING() << "ScreenRecorderWidget. Records directory is not available!";
        return;
    }

    _videoProcessor.setProgram(_ffmpegPathReader->read().toString());

    updatePropertiesInfo();

    if(_recordRect.isNull())
    {
        BMCL_WARNING() << "ScreenRecorderWidget. Null window widget!";
        return;
    }

    QString filename = QString("%1/%2.mp4").arg(_recordsPath).arg(QDateTime::currentDateTime().toString("yyyy-MM-dd_HHmmss"));
    if(!_sessionFileName.isEmpty())
    {
        filename = _sessionFileName;
    }

    QStringList list;
    list
        << "-framerate"
        << QString::number(_fpsReader->read().toInt())
#if defined(BMCL_PLATFORM_WINDOWS)
        << "-f"
        << "gdigrab"
        << "-i"
        << "desktop" //QString("title=%1").arg(windowname)
        << "-c:v"
        << _codecReader->read().toString()
#else
        << "-video_size"
        << QString("%1x%2").arg(_recordRect.width()).arg(_recordRect.height())
        << "-f"
        << "x11grab"
        << "-i"
        << QString(":0.0+%1,%2").arg(_recordRect.x()).arg(_recordRect.y())
#endif
        << filename;

    qDebug() << "ffmpeg args: " << list;

    _videoProcessor.setArguments(list);

    _videoProcessor.start();
    _state = State::Started;
    emit started();
}

void ScreenRecorder::stopRecord()
{
#if defined(BMCL_PLATFORM_WINDOWS)
    _videoProcessor.setProcessChannelMode(QProcess::ForwardedChannels);
    _videoProcessor.write("q");
    _videoProcessor.closeWriteChannel();
#else
    _videoProcessor.terminate();
#endif
    _state = State::Stopped;
    emit stopped();
}

void ScreenRecorder::pauseRecord()
{
#if defined(BMCL_PLATFORM_LINUX)
    if(_videoProcessor.state() == QProcess::Running)
    {
        if(!_isRecordPaused)
            kill(_videoProcessor.pid(), SIGSTOP);
        else
            kill(_videoProcessor.pid(), SIGCONT);

        _isRecordPaused = !_isRecordPaused;
        _state = State::Paused;
        emit paused();
    }
#elif defined(BMCL_PLATFORM_WINDOWS)

#endif
}

void ScreenRecorder::setRecordTime(const QTime& t)
{
    _recordTime = t;
    emit recordTimeChanged();
}

void ScreenRecorder::updateStorageInfo()
{
    if(!_storageInfo.isValid())
        return;

    _storageInfo.refresh();
    emit storageInfoChanged();
}

void ScreenRecorder::updatePropertiesInfo()
{
    QDesktopWidget* desktop = qApp->desktop();
    int w = 0;
    int h = 0;
    if(desktop)
    {
        for(int i = 0; i < desktop->screenCount(); ++i)
        {
            QRect rect = desktop->screenGeometry(i);
            w += rect.width();
            if(rect.height() > h)
                h = rect.height();
        }
    }
    _recordRect.setSize(QSize(w, h));

    _currentSettings = QString("Размер: (%1, %2), %3x%4").
                             arg(_recordRect.x()).
                             arg(_recordRect.y()).
                             arg(_recordRect.width()).
                             arg(_recordRect.height());
}

void ScreenRecorder::updateTimeInfo()
{
    if(_videoProcessor.state() != QProcess::Running)
        return;

    setRecordTime(_recordTime.addSecs(1));
}

void ScreenRecorder::timerEvent(QTimerEvent *event)
{
    static size_t i = 0;
    if(i % 5)
        updateStorageInfo();
    updateTimeInfo();
}
