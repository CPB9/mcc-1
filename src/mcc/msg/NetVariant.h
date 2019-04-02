#pragma once

#include "mcc/Config.h"
#include <bmcl/MemWriter.h>
#include <bmcl/MemReader.h>
#include <bmcl/Result.h>

#include <QString>
#include <QVariant>

#include <algorithm>
#include <string>
#include <type_traits>
#include <cstdint>
#include <cassert>

namespace mccmsg {

template <class... T>
struct AlignedUnion {
private:
#ifdef _MSC_VER
    typename std::aligned_union<0, T...>::type _data;
#else
    template <typename S>
    static constexpr S max(S t)
    {
        return t;
    }

    template <typename S, typename... A>
    static constexpr S max(S t, A... args)
    {
        return t > max(args...) ? t : max(args...);
    }

    alignas(max(alignof(T)...)) char _data[max(sizeof(T)...)];
#endif
};

enum class NetVariantType : std::uint8_t { None, Bool, Int, Uint, Float, Double, String, QString };
enum class NetVariantError { InvalidHeader, InvalidSize };

inline const char* toString(NetVariantType type)
{
    switch (type)
    {
    case NetVariantType::None:       return "none";
    case NetVariantType::Bool:       return "bool";
    case NetVariantType::Int:        return "int";
    case NetVariantType::Uint:       return "uint";
    case NetVariantType::Float:      return "float";
    case NetVariantType::Double:     return "double";
    case NetVariantType::String:     return "string";
    case NetVariantType::QString:    return "qstring";
    default: return "unknown";
    }
    return "unknown";
}

class MCC_MSG_DECLSPEC NetVariant {
public:
    inline NetVariant();
    inline NetVariant(bool value);
    inline NetVariant(std::int8_t value);
    inline NetVariant(std::int16_t value);
    inline NetVariant(std::int32_t value);
    inline NetVariant(std::int64_t value);
    inline NetVariant(std::uint8_t value);
    inline NetVariant(std::uint16_t value);
    inline NetVariant(std::uint32_t value);
    inline NetVariant(std::uint64_t value);
    inline NetVariant(float value);
    inline NetVariant(double value);
    inline NetVariant(const std::string& value);
    inline NetVariant(std::string&& value);
    inline NetVariant(const QString& value);
    inline NetVariant(QString&& value);
    NetVariant(const QVariant& value);
    inline NetVariant(const NetVariant& other);
    inline NetVariant(NetVariant&& other);
    inline ~NetVariant();

    // TODO: serialize, deserialize

    inline NetVariantType type() const;
    inline bool isNone() const;
    inline bool isSome() const;
    inline bool isBool() const;
    inline bool isFloat() const;
    inline bool isDouble() const;
    inline bool isInt() const;
    inline bool isUint() const;
    inline bool isString() const;
    inline bool isQString() const;

    inline bool asBool() const;
    inline std::int64_t asInt() const;
    inline std::uint64_t asUint() const;
    inline float asFloat() const;
    inline double asDouble() const;
    inline const std::string& asString() const;
    inline std::string&& asString();
    inline const QString& asQString() const;
    inline QString&& asQString();

    std::uint64_t toUint() const;
    std::int64_t toInt() const;
    float toFloat() const;
    double toDouble() const;
    QVariant toQVariant() const;

    std::string stringify() const;
    QString qstringify() const;

    void serialize(std::string* dest) const;
    std::string serialize() const;
    static bmcl::Result<NetVariant, NetVariantError> deserialize(bmcl::MemReader* src);

    NetVariant& operator=(const NetVariant& other);
    NetVariant& operator=(NetVariant&& other);

private:
    void destruct();
    void construct(const NetVariant& other);
    void construct(NetVariant&& other);

    template <typename T>
    T* cast();

    template <typename T, typename S, typename Q>
    T to(S stringConverter, Q qstringConverter) const;

    template <typename T>
    const T* cast() const;

    template <typename T>
    void init(T&& value, NetVariantType type);

    AlignedUnion<std::int64_t, std::uint64_t, float, double, std::string, QString> _data;
    NetVariantType _type;
};

template <typename T>
inline void NetVariant::init(T&& value, NetVariantType type)
{
    typedef typename std::decay<T>::type U;
    new (cast<U>()) U(std::forward<T>(value));
    _type = type;
}

inline NetVariant::NetVariant()
    : _type(NetVariantType::None)
{
}

inline NetVariant::NetVariant(bool value)
{
    init(value, NetVariantType::Bool);
}

inline NetVariant::NetVariant(std::int8_t value)
{
    init(std::int64_t(value), NetVariantType::Int);
}

inline NetVariant::NetVariant(std::int16_t value)
{
    init(std::int64_t(value), NetVariantType::Int);
}

inline NetVariant::NetVariant(std::int32_t value)
{
    init(std::int64_t(value), NetVariantType::Int);
}

inline NetVariant::NetVariant(std::int64_t value)
{
    init(value, NetVariantType::Int);
}

inline NetVariant::NetVariant(std::uint8_t value)
{
    init(std::uint64_t(value), NetVariantType::Uint);
}

inline NetVariant::NetVariant(std::uint16_t value)
{
    init(std::uint64_t(value), NetVariantType::Uint);
}

inline NetVariant::NetVariant(std::uint32_t value)
{
    init(std::uint64_t(value), NetVariantType::Uint);
}

inline NetVariant::NetVariant(std::uint64_t value)
{
    init(value, NetVariantType::Uint);
}

inline NetVariant::NetVariant(float value)
{
    init(value, NetVariantType::Float);
}

inline NetVariant::NetVariant(double value)
{
    init(value, NetVariantType::Double);
}

inline NetVariant::NetVariant(const std::string& value)
{
    init(value, NetVariantType::String);
}

inline NetVariant::NetVariant(std::string&& value)
{
    init(std::move(value), NetVariantType::String);
}

inline NetVariant::NetVariant(const QString& value)
{
    init(value, NetVariantType::QString);
}

inline NetVariant::NetVariant(QString&& value)
{
    init(std::move(value), NetVariantType::QString);
}

inline NetVariant::NetVariant(const NetVariant& other)
{
    construct(other);
}

inline NetVariant::NetVariant(NetVariant&& other)
{
    construct(std::move(other));
}

inline NetVariant::~NetVariant()
{
    destruct();
}

inline NetVariantType NetVariant::type() const
{
    return _type;
}

inline bool NetVariant::isNone() const
{
    return _type == NetVariantType::None;
}

inline bool NetVariant::isSome() const
{
    return _type != NetVariantType::None;
}

inline bool NetVariant::isBool() const
{
    return _type == NetVariantType::Bool;
}

inline bool NetVariant::isInt() const
{
    return _type == NetVariantType::Int;
}

inline bool NetVariant::isUint() const
{
    return _type == NetVariantType::Uint;
}

inline bool NetVariant::isFloat() const
{
    return _type == NetVariantType::Float;
}

inline bool NetVariant::isDouble() const
{
    return _type == NetVariantType::Double;
}

inline bool NetVariant::isString() const
{
    return _type == NetVariantType::String;
}

inline bool NetVariant::isQString() const
{
    return _type == NetVariantType::QString;
}

template <typename T>
inline T* NetVariant::cast()
{
    return reinterpret_cast<T*>(&_data);
}

template <typename T>
inline const T* NetVariant::cast() const
{
    return reinterpret_cast<const T*>(&_data);
}

inline bool NetVariant::asBool() const
{
    assert(isBool());
    return *cast<bool>();
}

inline std::int64_t NetVariant::asInt() const
{
    assert(isInt());
    return *cast<std::int64_t>();
}

inline std::uint64_t NetVariant::asUint() const
{
    assert(isUint());
    return *cast<std::uint64_t>();
}

inline float NetVariant::asFloat() const
{
    assert(isFloat());
    return *cast<float>();
}

inline double NetVariant::asDouble() const
{
    assert(isDouble());
    return *cast<double>();
}

inline const std::string& NetVariant::asString() const
{
    assert(isString());
    return *cast<std::string>();
}

inline std::string&& NetVariant::asString()
{
    assert(isString());
    return std::move(*cast<std::string>());
}

inline const QString& NetVariant::asQString() const
{
    assert(isQString());
    return *cast<QString>();
}

inline QString&& NetVariant::asQString()
{
    assert(isQString());
    return std::move(*cast<QString>());
}
}
