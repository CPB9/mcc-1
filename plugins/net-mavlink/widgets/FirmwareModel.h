#pragma once

#include <QAbstractTableModel>

#include "../Firmware.h"
#include "../device/Tm.h"

namespace mccmav {

class FirmwareModel : public QAbstractTableModel
{
public:
    explicit FirmwareModel(QObject* parent);

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void setFirmware(const FirmwarePtr& firmware);
    FirmwarePtr firmware() const;

    void setParam(const ParamValue& param);
    void setTmStorage(const bmcl::Rc<mccmsg::ITmStorage>& v);
private:
    FirmwarePtr      _firmware;
    bmcl::Rc<TmStorage> _tmStorage;

    std::vector<ParameterDescription> _params;
    std::vector<mccmsg::NetVariant> _values;
};
}
