#pragma once

#include <mcc/Config.h>
#include <mcc/map/Channel.h>

#include <atomic>
#include <mutex>
#include <thread>
#include <iostream>

namespace mccmap {

class ChannelConsumer {
public:
    inline ChannelConsumer();
    inline ~ChannelConsumer();

    template <typename T, typename C>
    void addWorker(Channel<T>* channel, C&& callable);
    inline void joinAll();
    inline void clear();

    ChannelConsumer(const ChannelConsumer&) = delete;
    ChannelConsumer(ChannelConsumer&&) = delete;
    ChannelConsumer& operator=(const ChannelConsumer&) = delete;
    ChannelConsumer& operator=(ChannelConsumer&&) = delete;

private:
    std::vector<std::thread> _workers;
};

inline ChannelConsumer::ChannelConsumer()
{
}

inline ChannelConsumer::~ChannelConsumer()
{
    joinAll();
}

template <typename T, typename C>
void ChannelConsumer::addWorker(Channel<T>* channel, C&& callable)
{
    _workers.emplace_back([channel, callable = std::forward<C>(callable)]() mutable {
        while (true) {
            bmcl::Option<T> data = channel->recv();
            if (data.isNone())
            {
                break;
            }
            callable(data.take());
        }
    });
}

inline void ChannelConsumer::joinAll()
{
    for (std::thread& worker : _workers) {
        worker.join();
    }
}

inline void ChannelConsumer::clear()
{
    joinAll();
    _workers.clear();
}
}
