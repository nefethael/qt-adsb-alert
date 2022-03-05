#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkCookieJar>
#include <QTimer>
#include <QGraphicsScene>
#include <QSettings>

#include "networknotifier.h"

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
    void initializeNotifier(const QSettings &settings);
    void restartConnection();
    void refreshScene();

    QList<Notifier*> m_notifierList;
    QNetworkAccessManager *m_manager = nullptr;
    QList<QUrl> m_tiles;
    QTimer *m_reqTimer = nullptr;

    QGraphicsScene  *m_scene;
    CraftModel*m_model;

    static const QUrl s_mainUrl;
};
#endif // MAINWINDOW_H
