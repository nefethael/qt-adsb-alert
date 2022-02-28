#ifndef CRAFTPROXYMODEL_H
#define CRAFTPROXYMODEL_H

#include <QSortFilterProxyModel>

class CraftProxyModel : public QSortFilterProxyModel
{
public:
    explicit CraftProxyModel(QObject *parent = nullptr);
    bool filterAcceptsRow(int source_row,
                          const QModelIndex &source_parent) const override;
public slots:
    void setCallsignFilter(QString filter);
    void setHexFilter(QString filter);
    void setCodeFilter(QString filter);
    void setTypeFilter(QString filter);
    void setFlagFilter(QString filter);
    void setAltMinFilter(QString filter);
    void setAltMaxFilter(QString filter);
    void setSpeedMinFilter(QString filter);
    void setSpeedMaxFilter(QString filter);
    void setRegistrationFilter(QString filter);
    void setSquawkFilter(QString filter);
    void setDistanceFilter(QString filter);

    void resetFilter();

private:
    QString m_callsignFilter;
    QString m_hexFilter;
    QString m_codeFilter;
    QString m_typeFilter;
    QString m_flagFilter;
    QString m_registrationFilter;
    QString m_squawkFilter;
    QString m_distanceFilter;

    QString m_altMinFilter;
    QString m_altMaxFilter;
    QString m_speedMinFilter;
    QString m_speedMaxFilter;
};

#endif // CRAFTPROXYMODEL_H
