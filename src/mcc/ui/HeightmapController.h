#pragma once

#include "mcc/Config.h"
#include "mcc/ui/QObjectRefCountable.h"
#include "mcc/hm/HmReader.h"
#include "mcc/plugin/PluginData.h"

#include <atomic>
#include <thread>

namespace mccui {

class HmSpinLock {
public:
// tas yielding spinlock
    HmSpinLock()
    {
        _lock.store(0, std::memory_order_relaxed);
    }

    inline void lock()
    {
        uint8_t value = 0;
        while (!_lock.compare_exchange_weak(value, 1, std::memory_order_acquire)) {
            value = 0;
            std::this_thread::yield();
        }
    }

    inline void unlock()
    {
        _lock.store(0, std::memory_order_release);
    }

private:
    std::atomic<std::uint8_t> _lock;
};

class MCC_UI_DECLSPEC HeightmapController : public QObjectRefCountable<QObject> {
    Q_OBJECT
public:
    HeightmapController(const mcchm::HmReader* reader);
    ~HeightmapController();

    void setHeightmapReader(const mcchm::HmReader* reader);

    mcchm::Rc<const mcchm::HmReader> cloneHeightmapReader() const;

signals:
    void heightmapReaderChanged(mcchm::Rc<const mcchm::HmReader> reader);

private:
    mutable HmSpinLock _lock;
    mcchm::Rc<const mcchm::HmReader> _reader;
};

class MCC_UI_DECLSPEC HmControllerPluginData : public mccplugin::PluginData {
public:
    static constexpr const char* id = "mcc::HmControllerPluginData";

    HmControllerPluginData(HeightmapController* hmReader);
    ~HmControllerPluginData();

    HeightmapController* hmReader();
    const HeightmapController* hmReader() const;

private:
    mcchm::Rc<HeightmapController> _hmController;
};
}
