#include <QApplication>
#include <QDir>
#include <QFontDatabase>
#include <QLibraryInfo>
#include <QSplashScreen>
#include <QTranslator>
#include <QThread>
#include <cstdio>
#include <ctime>
#include <memory>
#include <tclap/CmdLine.h>

#include <bmcl/Logging.h>
#include <bmcl/Utils.h>
#include <bmcl/Option.h>

#include "mcc/crashdump/CrashDump.h"
#include "mcc/path/Paths.h"
#include "mcc/plugin/Plugin.h"
#include "mcc/plugin/PluginCache.h"

#include "mcc/ide/view/MainWindow.h"

#ifdef _WIN32
#include <windows.h>
#endif

using namespace mccui;
using namespace mccui;

static void installTranslator(QApplication* app)
{
    QTranslator* qtTranslator = new QTranslator(app);
    if (qtTranslator->load("qt_ru", QApplication::applicationDirPath() + "/translations")) {
        app->installTranslator(qtTranslator);
        BMCL_INFO() << "загружен русский перевод: " + QApplication::applicationDirPath() + "/translations/qt_ru.qm";
    } else if (qtTranslator->load("qt_ru", QLibraryInfo::location(QLibraryInfo::TranslationsPath))) {
        app->installTranslator(qtTranslator);
        BMCL_INFO() << "загружен русский перевод: " + QLibraryInfo::location(QLibraryInfo::TranslationsPath) + "/qt_ru.qm";
    } else {
        BMCL_WARNING() << "не удалось загрузить русский перевод";
    }
}

void redirectStreamsToFile()
{
//    std::int64_t unixTime = std::time(NULL);
    QString logPath = mccpath::qGetLogsPath() + "/mcc." + QDateTime::currentDateTime().toString("yyyyMMdd.hhmmss.zzz") + ".log";
    QByteArray nativePath = logPath.toLocal8Bit();
    auto setupFile = [&nativePath](std::FILE* file) {
        std::fflush(file);
        std::freopen(nativePath.constData(), "a", file);
        std::setbuf(file, NULL);
    };
    setupFile(stdout);
    setupFile(stderr);
}

void prepareFonts()
{
    if(QFontDatabase::addApplicationFont(":fonts/RobotoCondensed-Regular.ttf") == -1)
    {
        BMCL_WARNING() << "Can't load font: RobotoCondensed-Regular.ttf";
    }
    if(QFontDatabase::addApplicationFont(":fonts/RobotoCondensed-Light.ttf") == -1)
    {
        BMCL_WARNING() << "Can't load font: RobotoCondensed-Light.ttf";
    }
    if(QFontDatabase::addApplicationFont(":fonts/RobotoCondensed-Bold.ttf") == -1)
    {
        BMCL_WARNING() << "Can't load font: RobotoCondensed-Bold.ttf";
    }
}

class DummyCleanupObject : public QObject {
public:
    DummyCleanupObject()
    {
        QApplication::postEvent(this, new QEvent(QEvent::MaxUser));
    }

    bool event(QEvent* event) override
    {
        QApplication::quit();
        return true;
    }
};

bmcl::Rc<mccplugin::PluginCacheWriter> processArgs(int argc, char* argv[], QApplication* app, std::unique_ptr<QSplashScreen>* splash)
{
    TCLAP::CmdLine cmdLine("mcc");
    TCLAP::MultiArg<std::string> pluginArg("p", "plugin", "Plugin", false, "path");

    std::vector<std::string> logValues = {"none", "panic", "critical", "warning", "info", "debug"};
    TCLAP::ValuesConstraint<std::string> allowedLogValues(logValues);

    TCLAP::ValueArg<std::string> logArg("l", "log-level", "Log level", false, "debug", &allowedLogValues);

    TCLAP::SwitchArg consoleArg("c", "console", "Log to console", false);

    TCLAP::ValueArg<int> threadsArg("t", "threads", "Max caf threads count", false, 0, "int");

    cmdLine.add(&pluginArg);
    cmdLine.add(&logArg);
    cmdLine.add(&consoleArg);
    cmdLine.parse(argc, argv);

    bmcl::LogLevel logLevel = bmcl::LogLevel::Debug;

    if (logArg.getValue() == "none") {
        logLevel = bmcl::LogLevel::None;
    } else if (logArg.getValue() == "panic") {
        logLevel = bmcl::LogLevel::Panic;
    } else if (logArg.getValue() == "critical") {
        logLevel = bmcl::LogLevel::Critical;
    } else if (logArg.getValue() == "warning") {
        logLevel = bmcl::LogLevel::Warning;
    } else if (logArg.getValue() == "info") {
        logLevel = bmcl::LogLevel::Info;
    } else if (logArg.getValue() == "debug") {
        logLevel = bmcl::LogLevel::Debug;
    }

    bool isConsole = consoleArg.getValue();
    if (!isConsole) {
#ifdef _WIN32
        FreeConsole();
#endif
        redirectStreamsToFile();
    }

    bmcl::Option<int> threads;
    if (threadsArg.isSet())
        threads = threadsArg.getValue();

    bmcl::setLogLevel(logLevel);
    BMCL_INFO() << "warning level set to " << logArg.getValue();

    auto ss = new QSplashScreen(QPixmap(":/app_icon.ico"));
    ss->show();
    app->processEvents();
    splash->reset(ss);

    setlocale(LC_ALL, "Rus");
    installTranslator(app);

    prepareFonts();

#ifdef MCC_BUILD_NUMBER
    BMCL_INFO() << "Build: " << MCC_BUILD_NUMBER;
    BMCL_INFO() << "Branch: " << MCC_BRANCH;
    BMCL_INFO() << "Commit: " << MCC_COMMIT_HASH;
#endif

    mccplugin::Rc<mccplugin::PluginCacheWriter> pluginCache = new mccplugin::PluginCacheWriter();

    QString pluginPath = QCoreApplication::applicationDirPath() + "/plugins";
    pluginCache->loadAllFromDir(pluginPath);

    for (const std::string& path : pluginArg.getValue()) {
        pluginCache->loadLibrary(QString::fromStdString(path));
    }
    pluginCache->initPlugins();

    return pluginCache;
}

int main(int argc, char* argv[])
{
    mcccrashdump::installProcessCrashHandlers();
    mcccrashdump::installThreadCrashHandlers();

#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    QApplication::setAttribute(Qt::AA_DisableHighDpiScaling, false);
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
#endif
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);

    int c = 1;
    QApplication app(c, argv);
    app.setApplicationName("Npu");
    app.setOrganizationName("Mcc");
    app.thread()->setObjectName("mcc.ui");
    QDir().mkpath(mccpath::qGetDataPath());
    QDir().mkpath(mccpath::qGetLogsPath());
    QDir().mkpath(mccpath::qGetUiPath());
    int result = -1;
    {
        std::unique_ptr<QSplashScreen> splash;
        mccplugin::Rc<mccplugin::PluginCacheWriter> pluginCache = processArgs(argc, argv, &app, &splash);
        mccide::MainWindow window(pluginCache.get());
        window.show();
        splash.release()->deleteLater();
        result = app.exec();
    }

    DummyCleanupObject obj;
    app.exec();

    DummyCleanupObject obj2;
    app.exec();

    return result;
}
