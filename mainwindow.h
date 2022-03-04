#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkCookieJar>
#include <QTimer>
#include <QGraphicsScene>

#include "craftmodel.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class CraftModel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void initDone();

private:
    Ui::MainWindow *ui;

    void replyFinished(QNetworkReply *reply);
    inline CraftModel* getCraftModel();

    void initializeTimers();
    void restartConnection();
    void refreshScene();
    bool sendMail(const Craft & craft);
    bool sendNotify(const Craft & craft);

    QNetworkAccessManager *m_manager = nullptr;
    QList<QUrl> m_tiles;
    QTimer *m_reqTimer = nullptr;

    QGraphicsScene  *m_scene;

    static const QUrl s_mainUrl;

    QString m_userSmtp;
    QString m_passSmtp;
    QString m_emailSmtp;
    QString m_notifyRun;
};
#endif // MAINWINDOW_H
