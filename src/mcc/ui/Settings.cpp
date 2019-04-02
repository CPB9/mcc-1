#include "mcc/ui/Settings.h"
#include "mcc/path/Paths.h"
#include "mcc/ui/CoordinateSystemController.h"

#include <bmcl/Logging.h>

#include <fmt/format.h>

#include <QApplication>
#include <QByteArray>
#include <QDebug>
#include <QDir>
#include <QSettings>
#include <QStandardPaths>
#include <QUuid>

namespace mccui {

Settings::Settings()
    : _qsettings(mccpath::qGetConfigPath() + QDir::separator() + "mcc-ui-ide.ini", QSettings::IniFormat)
{
}

Settings::~Settings()
{
}

bool Settings::contains(const QString& key) const
{
    return _qsettings.contains(key);
}

QVariant Settings::read(const QString& key, const QVariant& defaultValue) const
{
    return _qsettings.value(key, defaultValue);
}

Rc<SettingsReader> Settings::acquireReader(const QString& key, const QVariant& defValue) const
{
    return new SettingsReader(key, defValue, this);
}

bmcl::OptionRc<SettingsWriter> Settings::acquireUniqueWriter(const QString& key, const QVariant& defValue)
{
    auto it = _accessors.emplace(std::piecewise_construct, std::forward_as_tuple(key), std::forward_as_tuple());
    SettingsData& d = it.first->second;
    if (d.numSharedWriters || d.hasUniqueWriter) {
        return bmcl::None;
    }
    d.hasUniqueWriter = true;
    return Rc<SettingsWriter>(new SettingsWriter(key, defValue, this, true));
}

bmcl::OptionRc<SettingsWriter> Settings::acquireSharedWriter(const QString& key, const QVariant& defValue)
{
    auto it = _accessors.emplace(std::piecewise_construct, std::forward_as_tuple(key), std::forward_as_tuple());
    SettingsData& d = it.first->second;
    if (d.hasUniqueWriter) {
        return bmcl::None;
    }
    d.numSharedWriters++;
    return Rc<SettingsWriter>(new SettingsWriter(key, defValue, this, false));
}

void Settings::write(const QString& key, const QVariant& value)
{
    auto it = _accessors.find(key);
    if (it != _accessors.end()) {
        it->second.notifier.valueChanged(value);
    }
    _qsettings.setValue(key, value);
}

bool Settings::tryWrite(const QString& key, const QVariant& value)
{
    auto it = _accessors.find(key);
    if (it != _accessors.end()) {
        SettingsData& d = it->second;
        if (d.hasUniqueWriter) {
            BMCL_WARNING() << "failed to write settings: " << key;
            return false;
        }
        d.notifier.valueChanged(value);
    }
    _qsettings.setValue(key, value);
    return true;
}

void Settings::clear(const QString &key)
{
    _qsettings.remove(key);
}

const SettingsNotifier* Settings::getNotifier(const QString& key) const
{
    auto it = _accessors.emplace(std::piecewise_construct, std::forward_as_tuple(key), std::forward_as_tuple());
    return &it.first->second.notifier;
}

SettingsReader::SettingsReader(const QString& key, const QVariant& defValue, const Settings* settings)
    : _key(key)
    , _defaultValue(defValue)
    , _settings(settings)
{
}

SettingsReader::~SettingsReader()
{
}

QVariant SettingsReader::read() const
{
    return read(_defaultValue);
}

QVariant SettingsReader::read(const QVariant& defValue) const
{
    return _settings->read(_key, defValue);
}

void SettingsReader::setDefaultValue(const QVariant& defValue)
{
    _defaultValue = defValue;
}

const QString& SettingsReader::key() const
{
    return _key;
}

const QVariant& SettingsReader::defaultValue() const
{
    return _defaultValue;
}

SettingsWriter::SettingsWriter(const QString& key, const QVariant& defValue, Settings* settings, bool isUnique)
    : SettingsReader(key, defValue, settings)
    , _settings(settings)
    , _isUnique(isUnique)
{
}

SettingsWriter::~SettingsWriter()
{
    auto& map = _settings->_accessors;
    auto it = map.find(_key);
    if (it != map.end()) {
        Settings::SettingsData& d = it->second;
        if (_isUnique) {
            d.hasUniqueWriter = false;
        } else {
            d.numSharedWriters--;
        }
    }
}

void SettingsWriter::write(const QVariant& value)
{
    _settings->write(_key, value);
}

void SettingsWriter::writeDefault()
{
    _settings->write(_key, defaultValue());
}

void SettingsWriter::clear()
{
    _settings->clear(_key);
}

void Settings::restoreDefaults()
{
    _qsettings.clear();
    for (auto& it : _accessors) {
        //TODO: restore from default value passed as param
        it.second.notifier.valueChanged(_qsettings.value(it.first));
    }
}

/*
    "gps/settings/port"
    "gps/settings/speed"
    "gps/home/latitude"
    "gps/home/longitude"
    "gps/home/latitude"
    "gps/home/longitude"
*/

SettingsPluginData::SettingsPluginData(Settings* settings)
    : mccplugin::PluginData(SettingsPluginData::id)
    , _settings(settings)
{
}

SettingsPluginData::~SettingsPluginData()
{
}

Settings* SettingsPluginData::settings()
{
    return _settings.get();
}

const Settings* SettingsPluginData::settings() const
{
    return _settings.get();
}

std::size_t Settings::QStringHasher::operator()(const QString& key) const
{
    return qHash(key);
}
}
