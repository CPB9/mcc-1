#include "AirframesModel.h"
#include "mcc/res/Resource.h"

namespace mccmav {

AirframesModel::AirframesModel(QObject* parent) : QAbstractListModel(parent), _currentAirframe(-1)
{

}

void AirframesModel::addAiframeGroup(const QString& name, const QString& imageName, const QMap<int, QString>& airframes)
{
    QString imgPath = QString(":/net-mavlink/widgets/airframes/%1.svg").arg(imageName);
    QPixmap image = QPixmap::fromImage(mccres::renderSvg(imgPath, 64, 64));

    AirframeGroup group = { name, image, airframes };

    beginInsertRows(QModelIndex(), (int)_groups.size(), (int)_groups.size());
    _groups.emplace_back(std::move(group));
    endInsertRows();
}

int AirframesModel::rowCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
    return (int)_groups.size();
}

QVariant AirframesModel::data(const QModelIndex &index, int role /*= Qt::DisplayRole*/) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DisplayRole)
        return _groups[index.row()].name;

    if (role == Qt::BackgroundRole)
    {
        if (_groups[index.row()].airframes.contains(_currentAirframe))
            return QColor(Qt::yellow);
        return QColor(Qt::white);
    }

    if (role == AirframesModel::GroupImageRole)
        return _groups[index.row()].image;

    if (role == AirframesModel::AirframesMap)
        return QVariant::fromValue(_groups[index.row()].airframes);

    if (role == AirframesModel::GroupNameRole)
        return _groups[index.row()].name;

    if (role == AirframesModel::CurrentAirframe)
        return _currentAirframe;

    return QVariant();
}

void AirframesModel::setCurrentAirframe(int airframe)
{
    _currentAirframe = airframe;

    emit dataChanged(index(0, 0), index(rowCount()));
}

int AirframesModel::currentAirframe() const
{
    return _currentAirframe;
}
}

