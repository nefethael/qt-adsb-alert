#ifndef CRAFTMODEL_H
#define CRAFTMODEL_H

#include <QAbstractTableModel>
#include <QJsonDocument>
#include <QGeoCoordinate>
#include <QSet>
#include <QJSEngine>
#include <bitset>

#include "adsb.h"

enum CraftValidity{
    CraftValidity_pad73 = 3,
    CraftValidity_callsign_valid = CraftValidity_pad73,
    CraftValidity_baro_alt_valid,
    CraftValidity_geom_alt_valid,
    CraftValidity_position_valid,
    CraftValidity_gs_valid,
    CraftValidity_pad74,
    CraftValidity_ias_valid = CraftValidity_pad74,
    CraftValidity_tas_valid,
    CraftValidity_mach_valid,
    CraftValidity_track_valid,
    CraftValidity_track_rate_valid,
    CraftValidity_roll_valid,
    CraftValidity_mag_heading_valid,
    CraftValidity_true_heading_valid,
    CraftValidity_pad75,
    CraftValidity_baro_rate_valid = CraftValidity_pad75,
    CraftValidity_geom_rate_valid,
    CraftValidity_nic_a_valid,
    CraftValidity_nic_c_valid,
    CraftValidity_nic_baro_valid,
    CraftValidity_nac_p_valid,
    CraftValidity_nac_v_valid,
    CraftValidity_sil_valid,
    CraftValidity_pad76,
    CraftValidity_gva_valid = CraftValidity_pad76,
    CraftValidity_sda_valid,
    CraftValidity_squawk_valid,
    CraftValidity_emergency_valid,
    CraftValidity_spi_valid,
    CraftValidity_nav_qnh_valid,
    CraftValidity_nav_altitude_mcp_valid,
    CraftValidity_nav_altitude_fms_valid,
    CraftValidity_pad77,
    CraftValidity_nav_altitude_src_valid = CraftValidity_pad77,
    CraftValidity_nav_heading_valid,
    CraftValidity_nav_modes_valid,
    CraftValidity_alert_valid,
    CraftValidity_wind_valid,
    CraftValidity_temp_valid,
    CraftValidity_LAST,
};


class Craft
{
public:
    Craft(binCraft & bin, const QJsonDocument &icaoAircraftTypes, const QGeoCoordinate & home, QJSEngine & js);
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
    inline auto getGettingCloser() const { return m_gettingCloser;  }
    inline auto getSendAlert() const { return m_sendAlert;  }
    inline auto getSeen() const { return m_seen;  }
    inline auto getLastRefresh() const { return m_lastRefresh;  }
    inline auto getPos() const { return m_pos; }
    inline bool getValidity(CraftValidity type) const{
        if(type < CraftValidity_pad74){
            return m_pad73.test(type);
        }else if(type < CraftValidity_pad75){
            return m_pad74.test(type-CraftValidity_pad74);
        }else if(type < CraftValidity_pad76){
            return m_pad75.test(type-CraftValidity_pad75);
        }else if(type < CraftValidity_pad77){
            return m_pad76.test(type-CraftValidity_pad76);
        }else{
            return m_pad77.test(type-CraftValidity_pad77);
        }
    }

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
    qreal m_gettingCloser;
    bool m_sendAlert = false;
    float m_seen = 0.0;
    qint64 m_lastRefresh;
    QGeoCoordinate m_pos;

    std::bitset<8> m_pad73;
    std::bitset<8> m_pad74;
    std::bitset<8> m_pad75;
    std::bitset<8> m_pad76;
    std::bitset<8> m_pad77;
};

class CraftModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum Column{
      CM_CALLSIGN = 0,
      CM_HEX,
      CM_CODE,
      CM_TYPE,
      CM_FLAG,
      CM_ALT,
      CM_SPEED,
      CM_HEADING,
      CM_REG,
      CM_SQUAWK,
      CM_DIST,
      CM_AZIMUT
    };

    CraftModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void refreshCraft(QVector<binCraft>& c);
    void clearCraft();

    void setHome(QGeoCoordinate home);

    inline auto &getCraft() const { return m_craftData; }
    inline auto &getHome() const { return m_home; }

signals:
    void notify(const Craft & craft);

private:
    void prepareNotify(const Craft & craft);

    QVector<Craft> m_craftData;
    QJsonDocument m_icaoAircraftTypes;
    QSet<QString> m_alerted;

    QGeoCoordinate m_home;

    QJSEngine m_jsEngine;
};

#endif // CRAFTMODEL_H
