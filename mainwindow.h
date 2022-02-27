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

    QNetworkAccessManager *m_manager;

    static const QUrl s_mainUrl;
    static const QUrl s_bdxWUrl;
    static const QUrl s_bdxSUrl;
    static const QUrl s_bdxNUrl;
};
#endif // MAINWINDOW_H
