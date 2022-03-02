#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "adsb.h"
#include "craftmodel.h"
#include "craftproxymodel.h"

#include <QNetworkReply>
#include <QNetworkCookie>
#include <QTimer>
#include <QRandomGenerator>
#include <QUrlQuery>
#include <QSortFilterProxyModel>
#include <QSettings>

#define K_REFRESH_PERIOD_MS 10000

const QUrl MainWindow::s_mainUrl = QUrl("https://globe.adsbexchange.com/globeRates.json");

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
    return dynamic_cast<CraftModel*>(dynamic_cast<CraftProxyModel*>(ui->tableView->model())->sourceModel());
}

void MainWindow::restartConnection()
{
    if(m_manager){
        delete m_manager;
        m_manager = nullptr;
    }

    m_manager = new QNetworkAccessManager(this);
    auto cl = QList<QNetworkCookie>();
    cl.append(generateADSBCookie());

    for(auto &k : m_tiles){
        m_manager->cookieJar()->setCookiesFromUrl(cl, k);
    }

    connect(m_manager, &QNetworkAccessManager::finished, this, &MainWindow::replyFinished);

    QNetworkRequest initReq;
    initReq.setUrl(s_mainUrl);
    QUrlQuery query;
    query.addQueryItem("_", QString::number(QDateTime::currentMSecsSinceEpoch()));
    m_manager->get(initReq);
}

void MainWindow::initializeTimers()
{
    // when init is done
    connect(this, &MainWindow::initDone, this, [this](){

        // trigger adsb request every 10 seconds
        if(m_reqTimer){
            delete m_reqTimer;
            m_reqTimer = nullptr;
        }
        m_reqTimer = new QTimer(this);
        connect(m_reqTimer, &QTimer::timeout, this, [this](){
            if(!m_manager) {
                qDebug() << "Manager is not ready yet.";
            }
            getCraftModel()->clearCraft();

            // retrieve globe_x.binCraft
            for(auto k : m_tiles){
                QNetworkRequest req;
                req.setUrl(k);
                req.setRawHeader("Referer", "https://globe.adsbexchange.com/");
                m_manager->get(req);
            }
        });
        m_reqTimer->start(K_REFRESH_PERIOD_MS);
    });

    // Refresh cookie every 24H
    restartConnection();
    auto cookieTimer = new QTimer(this);
    connect(cookieTimer, &QTimer::timeout, this, &MainWindow::restartConnection);
    cookieTimer->start(86400000);
}



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QSettings settings("setup.ini", QSettings::Format::IniFormat);
    if(settings.status() != QSettings::Status::NoError){
        qDebug() << "Setup.ini not loaded";
    }

    //smtp
    auto userSmtp = settings.value("userSmtp").toString();
    auto passSmtp = settings.value("passSmtp").toString();
    auto emailSmtp = settings.value("emailSmtp").toString();
    //home
    auto home_lat = settings.value("home_lat").toDouble();
    auto home_lon = settings.value("home_lon").toDouble();
    auto home_alt = settings.value("home_alt").toDouble();
    // globe
    auto tiles = settings.value("tiles").toStringList();

    auto* model = new CraftModel(this);
    model->setSmtp(userSmtp, passSmtp, emailSmtp);
    model->setHome(QGeoCoordinate(home_lat, home_lon, home_alt));

    for(const auto &k : tiles){
        m_tiles.append(QUrl(QString("https://globe.adsbexchange.com/data/globe_%1.binCraft").arg(k)));
    }

    CraftProxyModel *proxyModel = new CraftProxyModel(this);
    proxyModel->setSourceModel(model);
    ui->tableView->setModel(proxyModel);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->sortByColumn(10, Qt::AscendingOrder);

    connect(ui->m_callsignLE, &QLineEdit::textEdited, proxyModel, &CraftProxyModel::setCallsignFilter);
    connect(ui->m_hexLE,      &QLineEdit::textEdited, proxyModel, &CraftProxyModel::setHexFilter);
    connect(ui->m_codeLE,     &QLineEdit::textEdited, proxyModel, &CraftProxyModel::setCodeFilter);
    connect(ui->m_typeLE,     &QLineEdit::textEdited, proxyModel, &CraftProxyModel::setTypeFilter);
    connect(ui->m_flagLE,     &QLineEdit::textEdited, proxyModel, &CraftProxyModel::setFlagFilter);
    connect(ui->m_altMinLE,   &QLineEdit::textEdited, proxyModel, &CraftProxyModel::setAltMinFilter);
    connect(ui->m_altMaxLE,   &QLineEdit::textEdited, proxyModel, &CraftProxyModel::setAltMaxFilter);
    connect(ui->m_speedMinLE, &QLineEdit::textEdited, proxyModel, &CraftProxyModel::setSpeedMinFilter);
    connect(ui->m_speedMaxLE, &QLineEdit::textEdited, proxyModel, &CraftProxyModel::setSpeedMaxFilter);
    connect(ui->m_regLE,      &QLineEdit::textEdited, proxyModel, &CraftProxyModel::setRegistrationFilter);
    connect(ui->m_squawkLE,   &QLineEdit::textEdited, proxyModel, &CraftProxyModel::setSquawkFilter);
    connect(ui->m_distLE,     &QLineEdit::textEdited, proxyModel, &CraftProxyModel::setDistanceFilter);
    connect(ui->m_resetPB,    &QPushButton::clicked, this, [this, proxyModel](){
        ui->m_callsignLE->setText("");
        ui->m_hexLE->setText("");
        ui->m_codeLE->setText("");
        ui->m_typeLE->setText("");
        ui->m_flagLE->setText("");
        ui->m_altMinLE->setText("");
        ui->m_altMaxLE->setText("");
        ui->m_speedMinLE->setText("");
        ui->m_speedMaxLE->setText("");
        ui->m_regLE->setText("");
        ui->m_squawkLE->setText("");
        ui->m_distLE->setText("");
        proxyModel->resetFilter();
    });

    // setup ADSB network manager
    initializeTimers();
}

MainWindow::~MainWindow()
{
    delete m_manager;
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

    qDebug() << nbCraft <<  " aircrafts fetched at " << QDateTime::currentDateTime().toString() ;

    QVector<struct binCraft> craftList(nbCraft);
    for(auto i = 0; i < nbCraft; i++){
        auto & craft = craftList[i];
        auto offset = (i * elementSize) + elementSize;
        memcpy((void*)&craft, bytes.data()+offset, elementSize);
    }
    getCraftModel()->refreshCraft(craftList);

    reply->deleteLater();
}
