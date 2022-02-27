#ifndef CRAFTMODEL_H
#define CRAFTMODEL_H

#include <QAbstractTableModel>
#include <QJsonDocument>
#include <QSet>

#include "adsb.h"

class Craft
{
public:
    Craft(binCraft & bin, const QJsonDocument &icaoAircraftTypes);
    Craft() = default;

    inline auto getCallsign() const { return m_callsign; };
    inline auto getHex() const { return m_hex; }
    inline auto getTypeCode() const { return m_typeCode; }
    inline auto getTypeDesc() const { return m_typeDesc; }
    inline auto getDbFlags() const { return m_dbFlags; }
    inline auto getAltitude() const { return m_altitude; }
    inline auto getGS() const { return m_groundSpeed; }
    inline auto getMach() const { return m_mach; }
    inline auto getIAS() const { return m_ias; }
    inline auto getHeading() const { return m_heading;  }
    inline auto getReg() const { return m_registration; }
    inline auto getSquawk() const { return m_squawk; }
    inline auto getDistanceToMe() const { return m_distanceToMe;  }
    inline auto getSendAlert() const { return m_sendAlert;  }
    inline auto getSeen() const { return m_seen;  }
    inline auto getLastRefresh() const { return m_lastRefresh;  }

private:
    QString m_callsign;
    QString m_hex;
    QString m_typeCode;
    QString m_typeDesc;
    QString m_dbFlags;
    quint32 m_altitude = 0;
    float m_groundSpeed = 0.0;
    float m_mach = 0.0;
    float m_ias = 0.0;
    float m_heading = 0.0;
    QString m_registration;
    QString m_squawk;
    qreal m_distanceToMe;
    bool m_sendAlert = false;
    float m_seen = 0.0;
    qint64 m_lastRefresh;
};

class CraftModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    CraftModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void refreshCraft(QVector<binCraft>& c);
    void clearCraft();
private:
    bool sendMailAlert(const Craft & craft);

    QVector<Craft> m_craftData;
    QJsonDocument m_icaoAircraftTypes;
    QSet<QString> m_alerted;
};

#endif // CRAFTMODEL_H
