#pragma once
#include <caf/actor.hpp>
#include "mcc/net/Exchanger.h"

namespace mccphoton {

class Exchanger : public mccnet::DefaultExchanger
{
public:
    Exchanger(const mccmsg::Channel& channel, const caf::actor& broker, const caf::actor& logger);
    mccnet::SearchResult find(const void * start, std::size_t size) override;
};
}
