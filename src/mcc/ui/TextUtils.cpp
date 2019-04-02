#include "mcc/ui/TextUtils.h"

#include <cmath>

namespace mccui {

QString bytesToString(uint64_t bytes, bool withSpace)
{
    // ГОСТ 8.417-2002, Приложение А
    const uint64_t denom(1024);
    const uint maxValue(999);
    const QString mask = withSpace ? QString("%1 %2") : QString("%1%2");
    double result(0);

    if(bytes < maxValue)
        return mask.arg(bytes).arg("Б");

    else if(bytes / denom < maxValue)
    {
        result = std::ceil((bytes * 10 / denom)) * 0.1;
        return mask.arg(result, 0, 'f', result > 100.0 ? 0 : 1).arg("КБ");
    }
    else if((bytes / (denom * denom)) < maxValue)
    {
        result = std::ceil((bytes * 10 / (denom * denom))) * 0.1;
        return mask.arg(result, 0, 'f', result > 100.0 ? 0 : 1).arg("МБ");
    }
    else if((bytes / (denom * denom * denom)) < maxValue)
    {
        result = std::ceil((bytes * 10 / (denom * denom * denom))) * 0.1;
        return mask.arg(result, 0, 'f', result > 100.0 ? 0 : 1).arg("ГБ");
    }
    else if((bytes / (denom * denom * denom * denom)) < maxValue)
    {
        result =std::ceil((bytes * 10 / (denom * denom * denom * denom))) * 0.1;
        return mask.arg(result, 0, 'f', result > 100.0 ? 0 : 1).arg("ТБ");
    }
    else
    {
        result = std::ceil((bytes * 10 / (denom * denom * denom * denom * denom))) * 0.1;
        return mask.arg(result, 0, 'f', result > 100.0 ? 0 : 1).arg("ПБ");
    }
}

QString shortTextLine(const QString& text, int maxSize)
{
    if (text.size() > maxSize)
        return text.left(maxSize) + "…";
    else
        return text;
}

QString shortTextLineInMiddle(const QString& text, int maxSize)
{
    if (text.size() > maxSize)
        return text.left(maxSize/2) + "…" + text.right(maxSize/2);
    else
        return text;
}

}
