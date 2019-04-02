#pragma once

#include <QObject>
#include "mcc/ui/Rc.h"
#include "mcc/ui/QObjectRefCountable.h"
#include "mcc/plugin/PluginData.h"

class QTextToSpeech;

namespace mccui {

class Settings;

class MCC_UI_DECLSPEC UserNotifier : public QObjectRefCountable<QObject>
{
    Q_OBJECT
public:
    UserNotifier(Settings* settings);
    Q_INVOKABLE void playSystemSound(const QString& name);
    Q_INVOKABLE void sayText(const QString& text);
    Q_INVOKABLE void activate();

    void playSound(const QString& path);
private:
    QTextToSpeech* _speech;
};

class MCC_UI_DECLSPEC UserNotifierPluginData : public mccplugin::PluginData {
public:
    static constexpr const char* id = "mcc::UserNotifierPluginData";

    UserNotifierPluginData(UserNotifier* settings);
    ~UserNotifierPluginData();

    UserNotifier* userNotifier();
    const UserNotifier* userNotifier() const;

private:
    Rc<UserNotifier> _notifier;
};

}