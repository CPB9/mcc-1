#pragma once

#include <QAbstractListModel>

#include <bmcl/Option.h>
#include <bmcl/OptionRc.h>

#include "mcc/msg/TmView.h"
#include "mcc/msg/exts/ErrStorage.h"

namespace mccuav {

class UavErrors : public QAbstractListModel
{
public:
    enum Roles
    {
        ErrorIdRole = Qt::UserRole + 1,
        HiddenRole
    };

    void setTmStorage(const bmcl::Rc<mccmsg::ITmStorage>& tmStorage);

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    bmcl::OptionRc<mccmsg::IErrStorage> _errorsStorage;
    bmcl::Option<mccmsg::HandlerId> _handler;
};

}
