#pragma once

#include <QObject>
#include <QPixmap>

#include <bmcl/Rc.h>

#include "mcc/Config.h"

#include "mcc/ui/QObjectRefCountable.h"

namespace mccuav {

class MCC_UAV_DECLSPEC UserParam : public mccui::QObjectRefCountable<QObject>
{
    Q_OBJECT
    Q_PROPERTY(QString info      READ info         WRITE setInfo         NOTIFY infoChanged)
public:
    UserParam();
    ~UserParam();
    const QString&  info() const;
    void setInfo(const QString& info);

    const QString& text() const;
    void setText(const QString& text);
signals:
    void infoChanged();
    void textChanged();
private:
    QString _info;
    QString _text;
};

using UserParamPtr = bmcl::Rc<UserParam>;
using UserParamPtrs= std::vector<UserParamPtr>;
}
