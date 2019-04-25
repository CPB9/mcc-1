#include "mcc/uav/UavErrors.h"
#include <bmcl/OptionPtr.h>
#include <QColor>

namespace mccuav {

void UavErrors::setTmStorage(const bmcl::Rc<mccmsg::ITmStorage>& tmStorage)
{
    _handler.clear();
    _errorsStorage = tmStorage->getExtension<mccmsg::IErrStorage>();
    if (_errorsStorage.isNone())
        return;

    auto errHandler = [this]() {
        //emit dataChanged(index(0,0), index(rowCount(), 0));
        beginResetModel();
        endResetModel();
    };
    _handler = _errorsStorage->addHandler(std::move(errHandler), true);
}

int UavErrors::rowCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
    if (_errorsStorage.isNone())
        return 0;
    return _errorsStorage->all().size();
}

QVariant UavErrors::data(const QModelIndex &index, int role /*= Qt::DisplayRole*/) const
{
    if (!index.isValid())
        return QVariant();

    if (_errorsStorage.isNone() || index.row() >= _errorsStorage->all().size())
        return QVariant();

    const auto id = std::next(_errorsStorage->all().begin(), index.row());
    auto msg = _errorsStorage->get(*id);
    if (msg.isNone())
        return QVariant();

    if (role == Roles::ErrorIdRole)
    {
        return QVariant::fromValue(msg->id());
    }
    if (role == Roles::HiddenRole)
    {
        return msg->isHidden();
    }
    if (role == Qt::DisplayRole)
    {
        if (msg->isHidden())
            return "[Скрыто]" + msg->qtext();
        return msg->qtext();
    }
    if (role == Qt::ForegroundRole)
    {
        switch (msg->level())
        {
            case bmcl::LogLevel::Panic:     return QColor(Qt::red);
            case bmcl::LogLevel::Critical:  return QColor(Qt::red);
            case bmcl::LogLevel::Warning:   return QColor(Qt::yellow);
            case bmcl::LogLevel::Info:      return QColor(Qt::white);
            case bmcl::LogLevel::Debug:
            case bmcl::LogLevel::None:     return QColor(Qt::gray);
        }
    }

    return QVariant();
}

void UavErrors::clearFilter()
{
    if(_errorsStorage.isNone())
        return;
    for(const auto& a : _errorsStorage->hidden())
        _errorsStorage->hide(a, false);
}

}
