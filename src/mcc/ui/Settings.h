#pragma once

#include <QString>
#include <QSettings>
#include <QPair>
#include <QSize>
#include <QPoint>
#include <QUuid>

#include <bmcl/Option.h>
#include <bmcl/OptionPtr.h>
#include <bmcl/OptionRc.h>

#include <unordered_map>

#include "mcc/plugin/PluginData.h"
#include "mcc/geo/Fwd.h"
#include "mcc/geo/LocalSystem.h"
#include "mcc/ui/Rc.h"
#include "mcc/ui/MapMode.h"
#include "mcc/ui/QObjectRefCountable.h"

class QByteArray;

namespace mccui {

typedef QPair<QString, int> SerialSettings;

class MCC_UI_DECLSPEC SettingsNotifier : public QObject {
    Q_OBJECT
signals:
    void valueChanged(const QVariant& value);
};

class Settings;

class MCC_UI_DECLSPEC SettingsReader : public RefCountable {
public:
    ~SettingsReader();

    QVariant read() const;
    QVariant read(const QVariant& defaultValue) const;
    void setDefaultValue(const QVariant& defaultValue);

    const QString& key() const;
    const QVariant& defaultValue() const;

    template<typename... A>
    QMetaObject::Connection onChange(A&&... args) const;

protected:
    friend class Settings;

    SettingsReader(const QString& key, const QVariant& defValue, const Settings* settings);

    QString _key;
    QVariant _defaultValue;

private:
    Rc<const Settings> _settings;
};

class MCC_UI_DECLSPEC SettingsWriter : public SettingsReader {
public:
    ~SettingsWriter();

    void write(const QVariant& value);
    void writeDefault();
    void clear();

private:
    friend class Settings;

    SettingsWriter(const QString& key, const QVariant& defValue, Settings* settings, bool isUnique);

    Rc<Settings> _settings; //TODO: remove duplicate
    bool _isUnique;
};

class MCC_UI_DECLSPEC Settings : public RefCountable {
public:
    Settings();
    ~Settings();

    bool contains(const QString& key) const; //TODO: remove method
    QVariant read(const QString& key, const QVariant& defaultValue = QVariant()) const;

    bool tryWrite(const QString& key, const QVariant& variant);
    void restoreDefaults();

    Rc<SettingsReader> acquireReader(const QString& key, const QVariant& defValue = QVariant()) const;
    bmcl::OptionRc<SettingsWriter> acquireUniqueWriter(const QString& key, const QVariant& defValue = QVariant());
    bmcl::OptionRc<SettingsWriter> acquireSharedWriter(const QString& key, const QVariant& defValue = QVariant());

    template <typename... A>
    QMetaObject::Connection onChange(const QString& key, A&&... args) const;

    MCC_DELETE_COPY_MOVE_CONSTRUCTORS(Settings)
private:
    friend class SettingsWriter;

    void write(const QString& key, const QVariant& variant);
    void clear(const QString& key);

    const SettingsNotifier* getNotifier(const QString& key) const;

    class QStringHasher {
    public:
        std::size_t operator()(const QString& str) const;
    };

    struct SettingsData {
        SettingsData()
            : numSharedWriters(0)
            , hasUniqueWriter(false)
        {
        }

        SettingsNotifier notifier;
        std::size_t numSharedWriters;
        bool hasUniqueWriter;
    };

    QSettings _qsettings;

    mutable std::unordered_map<QString, SettingsData, QStringHasher> _accessors;
};

template<typename... A>
inline QMetaObject::Connection Settings::onChange(const QString& key, A&&...args) const
{
    return QObject::connect(getNotifier(key), &SettingsNotifier::valueChanged, std::forward<A>(args)...);
}

template<typename... A>
QMetaObject::Connection SettingsReader::onChange(A&&... args) const
{
    return _settings->onChange(_key, std::forward<A>(args)...);
}

class MCC_UI_DECLSPEC SettingsPluginData : public mccplugin::PluginData {
public:
    static constexpr const char* id = "mccui::SettingsPluginData";

    SettingsPluginData(Settings* settings);
    ~SettingsPluginData();

    Settings* settings();
    const Settings* settings() const;

private:
    Rc<Settings> _settings;
};
}
