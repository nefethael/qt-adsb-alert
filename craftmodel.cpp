#include "craftmodel.h"

#include <QDebug>
#include <QFile>
#include <QTimer>
#include <QGeoCoordinate>
#include <cmath>
#include <QColor>
#include <QBrush>

#include "Smtp/smtpclient.h"
#include "Smtp/mimetext.h"

static QString stringFromChars(char* array, int size)
{
    if(array[0] == '\0'){
        return "N/A";
    }
    return QString::fromLocal8Bit(array, size).trimmed();
}


Craft::Craft(binCraft & bin, const QJsonDocument &icaoAircraftTypes)
{
    m_callsign = stringFromChars((char*)bin.callsign, 8);
    m_hex = QString("%1").arg(bin.hex, 8, 16).toUpper().trimmed();
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

    QGeoCoordinate aircraft(bin.lat/1e6, bin.lon/1e6, m_altitude*0.3048);
    QGeoCoordinate bordeaux(44.83, -0.53, 0);
    auto dist2d =  aircraft.distanceTo(bordeaux);
    m_distanceToMe = std::hypot(dist2d, aircraft.altitude() - bordeaux.altitude());

    m_sendAlert = ((m_distanceToMe < 50000)
            && ((bin.dbFlags & 1) // mili
                || (m_typeDesc.mid(1,1).toUInt() > 2) // plus de 2 réacteurs
                || m_callsign.startsWith("CTM")
                || m_callsign == "ZEROG"
                || m_typeCode == "A400"
                ));
}


CraftModel::CraftModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    QFile loadFile(":/adsb/icao_ac_types2");
    loadFile.open(QIODevice::ReadOnly|QIODevice::Text);
    QByteArray saveData = loadFile.readAll();
    m_icaoAircraftTypes = QJsonDocument::fromJson(saveData);
}

int CraftModel::rowCount(const QModelIndex & /*parent*/) const
{
   return m_craftData.size();
}

int CraftModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 11;
}

QVariant CraftModel::data(const QModelIndex &index, int role) const
{
    auto row = index.row();
    auto col = index.column();
    auto & craft = m_craftData[row];

    if (role == Qt::DisplayRole){
        switch(col){
        case 0:
            return craft.getCallsign();
        case 1:
            return craft.getHex();
        case 2:
            return craft.getTypeCode();
        case 3:
            return craft.getTypeDesc();
        case 4:
            return craft.getDbFlags();
        case 5:
            return craft.getAltitude();
        case 6:
            return craft.getGS();
        case 7:
            return craft.getHeading();
        case 8:
            return craft.getReg();
        case 9:
            return craft.getSquawk();
        case 10:
            return craft.getDistanceToMe();
        default:
            return QVariant();
        }
    }else if (role == Qt::BackgroundRole) {
        QColor color = craft.getDbFlags().isEmpty() ? Qt::white : Qt::yellow;
        return QBrush(color);
     }
    return QVariant();
}

QVariant CraftModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch(section){
        case 0:
            return QString("Callsign");
        case 1:
            return QString("ICAO");
        case 2:
            return QString("Code");
        case 3:
            return QString("Type");
        case 4:
            return QString("Flag");
        case 5:
            return QString("Altitude");
        case 6:
            return QString("Speed");
        case 7:
            return QString("Heading");
        case 8:
            return QString("Registration");
        case 9:
            return QString("Squawk");
        case 10:
            return QString("Distance");
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
        Craft craftToAdd(lst[i], m_icaoAircraftTypes);
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
            QTimer::singleShot(1000, [this, craftToAdd]() {
                sendMailAlert(craftToAdd);
            });
        }else{
            m_alerted.remove(craftToAdd.getHex());
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

bool CraftModel::sendMailAlert(const Craft & craft)
{
    if(m_alerted.contains(craft.getHex()))
    {
        qDebug() << "Already alerted" << craft.getHex();
        return false;
    }
    m_alerted.insert(craft.getHex());

#error to complete
    QString userSmtp = "<user@gmail.com>";
    QString passSmtp = "<two-factor app token>";
    QString userName = "<My Name>";

    SmtpClient smtp("smtp.gmail.com", 587, SmtpClient::TlsConnection);
    smtp.setUser(userSmtp);
    smtp.setPassword(passSmtp);

    MimeMessage message;
    message.setSender(new EmailAddress(userSmtp, userName));
    message.addRecipient(new EmailAddress(userSmtp, userName));
    message.setSubject(QString("ADSB ALERT %1").arg(craft.getTypeCode()));

    // Now add some text to the email.
    // First we create a MimeText object.
    QString str = QString("alt=%1 callsign=%2 flags=%3 dist=%4 gs=%5 hdg=%6 icao=%7 reg=%8 squawk=%9 type=%10 desc=%11\n")
        .arg(craft.getAltitude())
        .arg(craft.getCallsign())
        .arg(craft.getDbFlags())
        .arg(craft.getDistanceToMe())
        .arg(craft.getGS())
        .arg(craft.getHeading())
        .arg(craft.getHex())
        .arg(craft.getReg())
        .arg(craft.getSquawk())
        .arg(craft.getTypeCode())
        .arg(craft.getTypeDesc());

    MimeText text;
    text.setText(str);
    message.addPart(&text);

    smtp.connectToHost();
    smtp.login();
    bool ok = smtp.sendMail(message);
    smtp.quit();
    return ok;
}
