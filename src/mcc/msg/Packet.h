#pragma once
#include "mcc/Config.h"
#include <bmcl/Buffer.h>
#include "mcc/Rc.h"

namespace mccmsg {

    class MCC_MSG_DECLSPEC Packet : public mcc::RefCountable, public bmcl::Buffer {
    public:
        Packet();
        Packet(std::size_t size);
        Packet(const void* data, std::size_t size);
        Packet(bmcl::Bytes data);
        Packet(const Packet& other);
        Packet(Packet&& other);
        ~Packet();
    };

    using PacketPtr = mcc::Rc<Packet>;
}
