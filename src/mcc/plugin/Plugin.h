#pragma once

#include "mcc/plugin/Config.h"

#include <string>
#include <memory>

#define MCC_PLUGIN_MAIN_SYMBOL "mccCreatePlugin"

#ifdef _WIN32
# define MCC_PLUGIN_ENTRY_EXPORT __declspec(dllexport)
#else
# define MCC_PLUGIN_ENTRY_EXPORT __attribute__ ((__visibility__ ("default")))
#endif

#define MCC_INIT_PLUGIN(entry)                                                                                \
extern "C" {                                                                                                  \
MCC_PLUGIN_ENTRY_EXPORT void mccCreatePlugin(mccplugin::PluginCacheWriter* cache)                             \
{                                                                                                             \
    static_assert(std::is_same<decltype(&entry), mccplugin::PluginMain>::value, "Invalid plugin entry type"); \
    entry(cache);                                                                                             \
}                                                                                                             \
}

namespace mccplugin {

class PluginCache;
class PluginCacheWriter;

class MCC_PLUGIN_DECLSPEC Plugin {
public:
    virtual ~Plugin();

    const char* typeId() const;
    bool hasTypeId(const char* id) const;

    virtual int64_t priority() const;

protected:
    explicit Plugin(const char* typeId);

    friend class PluginCacheWriter;
    virtual bool init(PluginCache* cache);
    virtual void postInit(PluginCache* cache);

private:
    const char* _typeId;
};

typedef std::shared_ptr<Plugin> PluginPtr;
typedef void (*PluginMain)(PluginCacheWriter* cache);

}
