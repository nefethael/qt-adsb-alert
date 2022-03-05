#ifndef NETWORKNOTIFIER_H
#define NETWORKNOTIFIER_H

#include <QNetworkAccessManager>
#include <QSettings>
#include "craftmodel.h"

class Notifier : public QObject
{
    Q_OBJECT
public:
    Notifier(QObject * parent = nullptr);
    virtual ~Notifier();

    virtual bool sendNotification(const Craft & craft) = 0;
    virtual void setup(const QSettings & settings, CraftModel * origin) = 0;

protected:
    void replyFinished(QNetworkReply *reply);
    QNetworkAccessManager *m_manager = nullptr;
};

class SmtpNotifier : public Notifier
{
    Q_OBJECT
public:
    SmtpNotifier(QObject * parent = nullptr) : Notifier(parent){};

    virtual bool sendNotification(const Craft & craft) override;
    virtual void setup(const QSettings & settings, CraftModel * origin) override;

private:
    QString m_userSmtp;
    QString m_passSmtp;
    QString m_emailSmtp;
};

class NotifyRunNotifier : public Notifier
{
    Q_OBJECT
public:
    NotifyRunNotifier(QObject * parent = nullptr) : Notifier(parent){};

    virtual bool sendNotification(const Craft & craft) override;
    virtual void setup(const QSettings & settings, CraftModel * origin) override;

private:
    QString m_notifyRun;
};

class PushBulletNotifier : public Notifier
{
    Q_OBJECT
public:
    PushBulletNotifier(QObject * parent = nullptr) : Notifier(parent){};

    virtual bool sendNotification(const Craft & craft) override;
    virtual void setup(const QSettings & settings, CraftModel * origin) override;

private:
    QString m_pushBulletUser;
    QString m_pushBulletToken;
};

class TelegramNotifier : public Notifier
{
    Q_OBJECT
public:
    TelegramNotifier(QObject * parent = nullptr) : Notifier(parent){};

    virtual bool sendNotification(const Craft & craft) override;
    virtual void setup(const QSettings & settings, CraftModel * origin) override;
private:
    QString m_telegramChat;
    QString m_telegramToken;
};

#endif // NETWORKNOTIFIER_H
