#include "UserNotifier.h"

#include "mcc/path/Paths.h"

#include <QApplication>
#if QT_VERSION >= 0x050900
#include <QTextToSpeech>
#endif

#include <QSound>
#include "mcc/ui/WidgetUtils.h"

#include "bmcl/Logging.h"

namespace mccui {

UserNotifier::UserNotifier(Settings* settings)
{
#if QT_VERSION >= 0x050900
    _speech = new QTextToSpeech(this);
#endif
}

Q_INVOKABLE void UserNotifier::playSystemSound(const QString& name)
{
    playSound(mccpath::qGetSoundsPath() + "/" + name);
}

Q_INVOKABLE void UserNotifier::sayText(const QString& text)
{
#if QT_VERSION >= 0x050900
    _speech->say(text);
    _speech->setVolume(1.0);
#endif
}

Q_INVOKABLE void UserNotifier::activate()
{
    QApplication::alert(mccui::findMainWindow());
}

void UserNotifier::playSound(const QString& path)
{
    QSound::play(path);
}

UserNotifierPluginData::UserNotifierPluginData(UserNotifier* settings)
    : mccplugin::PluginData(UserNotifierPluginData::id)
        , _notifier(settings)
{
}

 UserNotifierPluginData::~UserNotifierPluginData()
{
}

mccui::UserNotifier* UserNotifierPluginData::userNotifier()
{
    return _notifier.get();
}

const mccui::UserNotifier* UserNotifierPluginData::userNotifier() const
{
    return _notifier.get();
}

}