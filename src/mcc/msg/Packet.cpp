#include "mcc/msg/Packet.h"

namespace mccmsg {
    Packet::Packet() : Buffer(){}
    Packet::Packet(std::size_t size) : Buffer(size) {}
    Packet::Packet(const void* data, std::size_t size) : Buffer(data, size) {}
    Packet::Packet(bmcl::Bytes data) : Buffer(data) {}
    Packet::Packet(const Packet& other) : Buffer(other) {}
    Packet::Packet(Packet&& other) : Buffer(std::move(other)) {}
    Packet::~Packet(){}
}