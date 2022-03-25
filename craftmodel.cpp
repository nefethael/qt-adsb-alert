#include "craftmodel.h"

#include <QDebug>
#include <QFile>
#include <QTimer>
#include <cmath>
#include <QColor>
#include <QBrush>
#include <QDateTime>
#include <bitset>

static QString stringFromChars(char* array, int size)
{
    if(array[0] == '\0'){
        return "N/A";
    }
    return QString::fromLocal8Bit(array, size).trimmed().remove(QChar('\u0000'));
}


Craft::Craft(binCraft & bin, const QJsonDocument &icaoAircraftTypes, const QGeoCoordinate & home, QJSEngine & js)
{
    m_callsign = stringFromChars((char*)bin.callsign, 8);
    m_hex = QString("%1").arg(bin.hex, 8, 16, QLatin1Char('0')).toUpper().trimmed();
    m_typeCode = stringFromChars((char*)bin.typeCode, 4);
    m_typeDesc = icaoAircraftTypes[m_typeCode][1].toString();
    m_dbFlags = bin.dbFlags;
    m_altitude = 25 * bin.baro_alt;
    m_groundSpeed = bin.gs / 10.0;
    m_mach = bin.mach / 1000.0;
    m_ias = static_cast<float>(bin.ias);
    m_heading = bin.true_heading / 90;
    m_registration = stringFromChars((char*)bin.registration, 12);
    m_squawk = QString("%1").arg(bin.squawk, 8, 16).toUpper().trimmed();
    m_seen = bin.seen / 10;
    m_lastRefresh = QDateTime::currentSecsSinceEpoch();

    m_pad73 = bin.pad73;
    m_pad74 = bin.pad74;
    m_pad75 = bin.pad75;
    m_pad76 = bin.pad76;
    m_pad77 = bin.pad77;

    QString flags;
    if (bin.dbFlags & 1){
        flags.append("MIL/");
    }
    if (bin.dbFlags & 2){
        flags.append("INT/");
    }
    if (bin.dbFlags & 4){
        flags.append("PIA/");
    }
    if (bin.dbFlags & 8){
        flags.append("LADD/");
    }
    m_dbFlags = flags;

    m_pos = QGeoCoordinate(bin.lat/1e6, bin.lon/1e6, m_altitude*0.3048);
    auto dist2d =  m_pos.distanceTo(home);
    m_distanceToMe = std::hypot(dist2d, m_pos.altitude() - home.altitude());
    m_gettingCloser = qAbs(m_pos.azimuthTo(home) - m_heading);

    js.globalObject().setProperty("callsign", m_callsign);
    js.globalObject().setProperty("hex", m_hex);
    js.globalObject().setProperty("typeCode", m_typeCode);
    js.globalObject().setProperty("typeDesc", m_typeDesc);
    js.globalObject().setProperty("dbFlags", m_dbFlags);
    js.globalObject().setProperty("altitude", m_altitude);
    js.globalObject().setProperty("groundSpeed", m_groundSpeed);
    js.globalObject().setProperty("mach", m_mach);
    js.globalObject().setProperty("ias", m_ias);
    js.globalObject().setProperty("heading", m_heading);
    js.globalObject().setProperty("registration", m_registration);
    js.globalObject().setProperty("squawk", m_squawk);
    js.globalObject().setProperty("distanceToMe", m_distanceToMe);
    js.globalObject().setProperty("gettingCloser", m_gettingCloser);

    QJSValue module = js.importModule("./sendAlert.mjs");
    QJSValue sendAlertFunction = module.property("sendAlert");
    QJSValue result = sendAlertFunction.call();

    if (result.isError()){
        qCritical()
                << "Uncaught exception at line"
                << result.property("lineNumber").toInt()
                << ":" << result.toString();
        m_sendAlert = false;
    }else{
        m_sendAlert = result.toBool();
    }
}


CraftModel::CraftModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    QFile loadFile(":/adsb/icao_ac_types2");
    loadFile.open(QIODevice::ReadOnly|QIODevice::Text);
    QByteArray saveData = loadFile.readAll();
    m_icaoAircraftTypes = QJsonDocument::fromJson(saveData);
    m_jsEngine.installExtensions(QJSEngine::ConsoleExtension);
}

int CraftModel::rowCount(const QModelIndex & /*parent*/) const
{
   return m_craftData.size();
}

int CraftModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 12;
}

QVariant CraftModel::data(const QModelIndex &index, int role) const
{
    auto row = index.row();
    auto col = index.column();
    auto & craft = m_craftData[row];

    if (role == Qt::DisplayRole){
        switch(col){
        case CM_CALLSIGN:
            return craft.getCallsign();
        case CM_HEX:
            return craft.getHex();
        case CM_CODE:
            return craft.getTypeCode();
        case CM_TYPE:
            return craft.getTypeDesc();
        case CM_FLAG:
            return craft.getDbFlags();
        case CM_ALT:
            return craft.getAltitude();
        case CM_SPEED:
            return craft.getGS();
        case CM_HEADING:
            return craft.getHeading();
        case CM_REG:
            return craft.getReg();
        case CM_SQUAWK:
            return craft.getSquawk();
        case CM_DIST:
            return craft.getDistanceToMe();
        case CM_AZIMUT:
            return craft.getGettingCloser();
        default:
            return QVariant();
        }
    }else if (role == Qt::BackgroundRole) {
        QColor color = craft.getSendAlert() ? Qt::yellow : Qt::white;
        return QBrush(color);
     }
    return QVariant();
}

QVariant CraftModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch(section){
        case CM_CALLSIGN:
            return QString("Callsign");
        case CM_HEX:
            return QString("ICAO");
        case CM_CODE:
            return QString("Code");
        case CM_TYPE:
            return QString("Type");
        case CM_FLAG:
            return QString("Flag");
        case CM_ALT:
            return QString("Altitude");
        case CM_SPEED:
            return QString("Speed");
        case CM_HEADING:
            return QString("Heading");
        case CM_REG:
            return QString("Registration");
        case CM_SQUAWK:
            return QString("Squawk");
        case CM_DIST:
            return QString("Distance");
        case CM_AZIMUT:
            return QString("Azimut");
        default:
            QVariant();
        }
    }
    return QVariant();
}

void CraftModel::refreshCraft(QVector<binCraft>& lst)
{
    beginResetModel();
    for(auto i = 0; i < lst.size(); i++){
        Craft craftToAdd(lst[i], m_icaoAircraftTypes, m_home, m_jsEngine);
        bool isFound = false;
        for (auto j = 0; (j < m_craftData.size()) && !isFound; j++){
            if(m_craftData[j].getHex() == craftToAdd.getHex()){
                m_craftData[j] = craftToAdd;
                isFound = true;
            }
        }
        if(!isFound){
            m_craftData.append(craftToAdd);
        }
        if(craftToAdd.getSendAlert()){
            prepareNotify(craftToAdd);
        }
    }
    endResetModel();
}

void CraftModel::clearCraft()
{
    auto nowsec = QDateTime::currentSecsSinceEpoch();

    beginResetModel();
    m_craftData.erase(std::remove_if(m_craftData.begin(), m_craftData.end(), [this, nowsec](const Craft & elt){
        auto res = (elt.getSeen() > 1000)
                || (nowsec - elt.getLastRefresh() > 1000);
        if(res){
            qDebug() << "Removing " << elt.getHex() << elt.getCallsign() << elt.getSeen() << nowsec << elt.getLastRefresh();
            m_alerted.remove(elt.getHex());
        }
        return res;
    }), m_craftData.end());

    endResetModel();
}

void CraftModel::prepareNotify(const Craft & craft)
{
    if(m_alerted.contains(craft.getHex()))
    {
        qDebug() << "Already alerted" << craft.getHex();
        return;
    }
    m_alerted.insert(craft.getHex());

    notify(craft);
}

void CraftModel::setHome(QGeoCoordinate home)
{
    m_home = home;
}
