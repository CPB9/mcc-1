#pragma once

#include <bmcl/TimeUtils.h>

#include <QDateTime>

#include <string>

namespace mccdb {

inline std::string serializeTime(const bmcl::SystemTime& t, const char* format = "yyyy-MM-dd HH:mm:ss.zzz")
{
    return QDateTime::fromMSecsSinceEpoch(bmcl::toMsecs(t.time_since_epoch()).count()).toString(format).toStdString();
}

inline bmcl::SystemTime deserializeTime(bmcl::StringView time, const char* format = "yyyy-MM-dd HH:mm:ss.zzz")
{
    QDateTime qtime = QDateTime::fromString(QString::fromUtf8(time.data(), time.size()), QString(format));
    return bmcl::SystemTime(std::chrono::duration_cast<bmcl::SystemTime::duration>(std::chrono::milliseconds(qtime.toMSecsSinceEpoch())));
}

}
