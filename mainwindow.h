#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkCookieJar>

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
    void initializeADSB();
    inline CraftModel* getCraftModel();
    void setADSBCookie();

    QNetworkAccessManager *m_manager;
    QList<QUrl> m_tiles;

    static const QUrl s_mainUrl;
};
#endif // MAINWINDOW_H
