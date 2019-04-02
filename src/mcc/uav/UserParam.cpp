#include "mcc/uav/UserParam.h"

#include <QApplication>

namespace mccuav {

UserParam::UserParam()
{

}

UserParam::~UserParam()
{

}

const QString& UserParam::info() const
{
    return _info;
}

void UserParam::setInfo(const QString& info)
{
    _info = info;
}

const QString& UserParam::text() const
{
    return _text;
}

void UserParam::setText(const QString& text)
{
    _text = text;
    emit textChanged();
}

}
