#include "networknotifier.h"
#include <QNetworkReply>

Notifier::Notifier(QObject * parent) : QObject(parent)
{
    m_manager = new QNetworkAccessManager(this);
    connect(m_manager, &QNetworkAccessManager::finished, this, &Notifier::replyFinished);
}

Notifier::~Notifier()
{
    delete m_manager;
}

void NotifyRunNotifier::setup(const QSettings & settings, CraftModel * origin)
{
    m_notifyRun = settings.value("notifyrun").toString();

    if(m_notifyRun.isEmpty()){
        qInfo() << "No NotifyRun information, don't notify!";
    }else{
        connect(origin, &CraftModel::notify, this, &NotifyRunNotifier::sendNotification);
    }
}

bool NotifyRunNotifier::sendNotification(const Craft & craft)
{
    QString str = QString("ALERT ADSB %1 %2\n")
        .arg(craft.getCallsign())
        .arg(craft.getTypeCode());

    QNetworkRequest req;
    req.setUrl(QString("https://notify.run/%1").arg(m_notifyRun));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    m_manager->post(req, str.toUtf8());

    return true;
}

void TelegramNotifier::setup(const QSettings & settings, CraftModel * origin)
{
    m_telegramChat = settings.value("telegram_chat").toString();
    m_telegramToken = settings.value("telegram_token").toString();

    if(m_telegramToken.isEmpty()){
        qInfo() << "No Telegram information, don't notify!";
    }else{
        connect(origin, &CraftModel::notify, this, &TelegramNotifier::sendNotification);
    }
}

bool TelegramNotifier::sendNotification(const Craft & craft)
{
    QString text = QString("[%1](https://globe.adsbexchange.com/?icao=%1) \\| Type:*%2* \\| Callsign:`%3` \\| Reg:`%4`\n")
        .arg(craft.getHex())
        .arg(craft.getTypeCode())
        .arg(craft.getCallsign())
        .arg(craft.getReg());

    QString str = QString("{\"chat_id\":\"%1\", \"text\": \"%2\", \"disable_web_page_preview\": \"true\", \"parse_mode\": \"Markdown\" }")
        .arg(m_telegramChat)
        .arg(text);

    QNetworkRequest req;
    req.setUrl(QString("https://api.telegram.org/bot%1/sendMessage").arg(m_telegramToken));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    m_manager->post(req, str.toUtf8());

    return true;
}

void PushBulletNotifier::setup(const QSettings & settings, CraftModel * origin)
{
    m_pushBulletUser = settings.value("pushbullet_user").toString();
    m_pushBulletToken = settings.value("pushbullet_token").toString();

    if(m_pushBulletToken.isEmpty()){
        qInfo() << "No PushBullet information, don't notify!";
    }else{
        connect(origin, &CraftModel::notify, this, &PushBulletNotifier::sendNotification);
    }
}

bool PushBulletNotifier::sendNotification(const Craft & craft)
{
    QString str = QString("{\"email\":\"%1\", \"type\":\"note\", \"title\": \"ADSB ALERT\", \"body\": \"%2 %3\"}")
        .arg(m_pushBulletUser)
        .arg(craft.getCallsign())
        .arg(craft.getTypeCode());

    QNetworkRequest req;
    req.setUrl(QString("https://api.pushbullet.com/v2/pushes"));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setRawHeader("Access-Token", m_pushBulletToken.toUtf8());
    m_manager->post(req, str.toUtf8());

    return true;
}

void Notifier::replyFinished(QNetworkReply *reply)
{
    if (reply->operation() == QNetworkAccessManager::PostOperation){
        qInfo() << "Post OK for" << reply->request().url();
    }
    reply->deleteLater();
}


