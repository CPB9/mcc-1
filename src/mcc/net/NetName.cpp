#include "mcc/net/NetName.h"
#include "QtNetwork/QNetworkInterface"


namespace mccnet {

bmcl::Option<uint32_t> netName()
{
    uint32_t owner = 0;
    QString owner_mac;
    for (const QNetworkInterface& i : QNetworkInterface::allInterfaces())
    {
        QString name = i.hardwareAddress();
        name.remove(":");
        QString shortname = name.right(8);
        uint32_t supposed_name = shortname.toUInt(nullptr, 16);
        owner = std::max(owner, supposed_name);
        if (supposed_name == owner && owner != 0)
            owner_mac = name;
    }
    if (owner == 0)
        return bmcl::None;
    return owner;
}

}
