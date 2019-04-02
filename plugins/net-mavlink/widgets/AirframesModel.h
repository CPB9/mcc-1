#pragma once

#include <QAbstractListModel>
#include <QPixmap>

namespace mccmav {

struct AirframeGroup
{
    QString name;
    QPixmap image;
    QMap<int, QString> airframes;
};

class AirframesModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles
    {
        GroupImageRole = Qt::UserRole + 1,
        AirframesMap,
        GroupNameRole,
        CurrentAirframe
    };

    AirframesModel(QObject* parent);
    void addAiframeGroup(const QString& name, const QString& imageName, const QMap<int, QString>& airframes);
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void setCurrentAirframe(int airframe);
    int currentAirframe() const;
private:
    std::vector<AirframeGroup> _groups;
    int _currentAirframe;
};
}
