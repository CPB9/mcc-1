#include "mcc/plugin/PluginCache.h"
#include "mcc/plugin/Plugin.h"
#include "mcc/plugin/PluginData.h"

#include <bmcl/Logging.h>

#include <assert.h>

#include <QLibrary>
#include <QFileInfoList>
#include <QString>
#include <QDir>

namespace mccplugin {

PluginCache::PluginCache()
{
}

PluginCache::~PluginCache()
{
}

PluginData* PluginCache::findData(const char* name)
{
    auto it = _pluginData.find(std::string(name));
    if (it == _pluginData.end()) {
        return nullptr;
    }
    return it->second.get();
}

PluginCacheWriter::PluginCacheWriter()
{
}

PluginCacheWriter::~PluginCacheWriter()
{
    clear();
}

void PluginCacheWriter::loadAllFromDir(const QString& path)
{
#ifdef _WIN32
    QFileInfoList lst = QDir(path).entryInfoList(QStringList() << "*.dll");
#else
    QFileInfoList lst = QDir(path).entryInfoList(QStringList() << "*.so");
#endif
    for (const QFileInfo& info : lst) {
        loadLibrary(info.absoluteFilePath());
    }
}

bool PluginCacheWriter::loadLibrary(const QString& path)
{
    QLibrary lib;
    lib.setFileName(path);
    bmcl::Logger logger(bmcl::LogLevel::Debug);
    logger << "Loading plugin " << path << "... ";
    if (!lib.load()) {
        logger << "FAILED to load (" << lib.errorString() << ")";
        return false;
    }
    mccplugin::PluginMain symbol = (mccplugin::PluginMain)lib.resolve(MCC_PLUGIN_MAIN_SYMBOL);
    if (!symbol) {
        logger << "FAILED to find init symbol '" MCC_PLUGIN_MAIN_SYMBOL << "'";
        return false;
    }
    logger << "OK";
    symbol(this);
    return true;
}

void PluginCacheWriter::addPlugin(const PluginPtr& plugin)
{
    assert(plugin);
    _plugins.push_back(plugin);
}

void PluginCacheWriter::addPlugin(PluginPtr&& plugin)
{
    assert(plugin);
    _plugins.push_back(std::move(plugin));
}

bool PluginCache::addPluginData(std::unique_ptr<PluginData>&& data)
{
    auto it = _pluginData.emplace(std::string(data->dataId()), std::move(data));
    assert(it.second);
    return it.second;
}

void PluginCacheWriter::sortByPriority()
{
    std::sort(_plugins.begin(), _plugins.end(), [](const PluginPtr& left, const PluginPtr& right) {
        return left->priority() < right->priority();
    });

}

void PluginCacheWriter::initPlugins()
{
    sortByPriority();

    std::vector<PluginPtr> uninitialized = _plugins;
    std::vector<PluginPtr> initialized;
    while (true) {
        std::vector<PluginPtr> currentUnitialized;
        currentUnitialized.reserve(uninitialized.size());
        for (const PluginPtr& plugin : uninitialized) {
            if (!plugin->init(this)) {
                currentUnitialized.push_back(plugin);
            } else {
                initialized.push_back(plugin);
            }
        }

        if (currentUnitialized.empty() || (uninitialized.size() == currentUnitialized.size())) {
            uninitialized = std::move(currentUnitialized);
            break;
        }
        uninitialized = std::move(currentUnitialized);
    }
    _plugins = std::move(initialized);
    sortByPriority();

    if (!uninitialized.empty()) {
        BMCL_CRITICAL() << "could not initialize " << uninitialized.size() << " plugins";
        for (const PluginPtr& plugin : uninitialized) {
            BMCL_DEBUG() << "failed to init plugin: " << plugin->typeId();
        }
    } else {
        BMCL_INFO() << "initialized all plugins";
    }

    for (const PluginPtr& plugin : _plugins) {
        plugin->postInit(this);
    }
}

void PluginCacheWriter::clear()
{
    _plugins.clear();
    _pluginData.clear();
}

}
