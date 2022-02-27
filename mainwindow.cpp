#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "adsb.h"
#include "craftmodel.h"

#include <QNetworkReply>
#include <QNetworkCookie>
#include <QTimer>
#include <QRandomGenerator>
#include <QUrlQuery>
#include <QSortFilterProxyModel>

const QUrl MainWindow::s_mainUrl = QUrl("https://globe.adsbexchange.com/globeRates.json");
const QUrl MainWindow::s_bdxWUrl = QUrl("https://globe.adsbexchange.com/data/globe_0016.binCraft");
const QUrl MainWindow::s_bdxSUrl = QUrl("https://globe.adsbexchange.com/data/globe_6384.binCraft");
const QUrl MainWindow::s_bdxNUrl = QUrl("https://globe.adsbexchange.com/data/globe_6505.binCraft");

static QString JSMathRandomToString36(double input)
{
    static const auto clistarr = QByteArray("0123456789abcdefghijklmnopqrstuvwxyz");
    QString result;

    for(auto i = 0; i < 11 ; i++){
        auto tempVal = (input * 36);
        auto tempValStr = QString::number(tempVal, 'R');
        auto decIndex = tempValStr.indexOf(".");
        if (decIndex > 0) {
            auto topVal = tempValStr.mid(0, decIndex).toShort();
            result.append(clistarr[topVal]);
            input = tempVal - topVal;
        }
    }
    return (result);
}

static QNetworkCookie generateADSBCookie()
{
    quint64 ts = QDateTime::currentMSecsSinceEpoch() + 2*86400*1000;
    auto nextDate = QDateTime::currentDateTime().addMSecs(2*24*60*60*1000);
    QString tmp = JSMathRandomToString36(QRandomGenerator().global()->generateDouble());
    auto cookId = QString("%1_%2").arg(ts).arg(tmp);
    qDebug() << "generating cookie" << cookId.toUtf8();
    QNetworkCookie cc("adsbx_sid", cookId.toUtf8());
    cc.setExpirationDate(nextDate);
    cc.setDomain("globe.adsbexchange.com");
    cc.setPath("/");
    cc.setHttpOnly(false);
    cc.setSecure(false);
    return cc;
}

CraftModel* MainWindow::getCraftModel()
{
    return dynamic_cast<CraftModel*>(dynamic_cast<QSortFilterProxyModel*>(ui->tableView->model())->sourceModel());
}

void MainWindow::initializeADSB()
{
    m_manager = new QNetworkAccessManager(this);

    auto cl = QList<QNetworkCookie>();
    cl.append(generateADSBCookie());
    m_manager->cookieJar()->setCookiesFromUrl(cl, s_bdxWUrl);
    m_manager->cookieJar()->setCookiesFromUrl(cl, s_bdxSUrl);
    m_manager->cookieJar()->setCookiesFromUrl(cl, s_bdxNUrl);

    connect(m_manager, &QNetworkAccessManager::finished, this, &MainWindow::replyFinished);

    // when init is done
    connect(this, &MainWindow::initDone, this, [this](){

        // trigger adsb request every 10 seconds
        auto timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, [this](){

            getCraftModel()->clearCraft();

            // retrieve globe_x.binCraft
            QNetworkRequest req;
            req.setUrl(s_bdxWUrl);
            req.setRawHeader("Referer", "https://globe.adsbexchange.com/");
            m_manager->get(req);

            QNetworkRequest req2;
            req2.setUrl(s_bdxSUrl);
            req2.setRawHeader("Referer", "https://globe.adsbexchange.com/");
            m_manager->get(req2);

            QNetworkRequest req3;
            req3.setUrl(s_bdxNUrl);
            req3.setRawHeader("Referer", "https://globe.adsbexchange.com/");
            m_manager->get(req3);

        });
        timer->start(10000);
    });

    QNetworkRequest initReq;
    initReq.setUrl(s_mainUrl);
    QUrlQuery query;
    query.addQueryItem("_", QString::number(QDateTime::currentMSecsSinceEpoch()));
    m_manager->get(initReq);
}



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(new CraftModel(this));
    ui->tableView->setModel(proxyModel);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->sortByColumn(10, Qt::AscendingOrder);

    // setup ADSB network manager
    initializeADSB();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::replyFinished(QNetworkReply *reply)
{
    auto url = reply->request().url();
    if(url == s_mainUrl){
        emit initDone();
        return ;
    }

    auto bytes = reply->readAll();
    if ((bytes.size() < 7) || (bytes.mid(0,6).toStdString() == "<html>")){
        qDebug() << "Problem" << reply->errorString();
        return;
    }

    auto elementSize = (int)sizeof(struct binCraft);
    auto nbCraft = (bytes.count() / elementSize) - 1;

    struct binHeader hdr;
    memcpy((void*)&hdr, bytes.data(), sizeof(struct binHeader));

    qDebug() << "nb avions" << nbCraft;

    QVector<struct binCraft> craftList(nbCraft);
    for(auto i = 0; i < nbCraft; i++){
        auto & craft = craftList[i];
        auto offset = (i * elementSize) + elementSize;
        memcpy((void*)&craft, bytes.data()+offset, elementSize);
    }
    getCraftModel()->refreshCraft(craftList);
}
