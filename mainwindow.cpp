#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "adsb.h"
#include "craftmodel.h"
#include "craftproxymodel.h"
#include "networknotifier.h"

#include <QNetworkReply>
#include <QNetworkCookie>
#include <QTimer>
#include <QRandomGenerator>
#include <QUrlQuery>
#include <QSortFilterProxyModel>
#include <QGraphicsEllipseItem>

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
    m_manager->get(initReq);
}

void MainWindow::initializeNotifier(const QSettings &settings)
{
    m_notifierList.append(new SmtpNotifier(this));
    m_notifierList.append(new NotifyRunNotifier(this));
    m_notifierList.append(new PushBulletNotifier(this));
    m_notifierList.append(new TelegramNotifier(this));

    for(auto & n : m_notifierList){
        n->setup(settings, m_model);
    }
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
                qInfo() << "Manager is not ready yet.";
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
    , m_scene(new QGraphicsScene(this))
{
    ui->setupUi(this);
    ui->graphicsView->setScene(m_scene);

    QSettings settings("setup.ini", QSettings::Format::IniFormat);
    if(settings.status() != QSettings::Status::NoError){
        qCritical() << "Setup.ini not loaded";
    }

    //home
    auto home_lat = settings.value("home_lat").toDouble();
    auto home_lon = settings.value("home_lon").toDouble();
    auto home_alt = settings.value("home_alt").toDouble();
    // globe
    auto tiles = settings.value("tiles").toStringList();

    m_model = new CraftModel(this);
    m_model->setHome(QGeoCoordinate(home_lat, home_lon, home_alt));

    for(const auto &k : tiles){
        m_tiles.append(QUrl(QString("https://globe.adsbexchange.com/data/globe_%1.binCraft").arg(k)));
    }

    CraftProxyModel *proxyModel = new CraftProxyModel(this);
    proxyModel->setSourceModel(m_model);
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

    // setup notifier
    initializeNotifier(settings);
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
        reply->deleteLater();
        return ;
    }
    if (reply->operation() == QNetworkAccessManager::PostOperation){
        reply->deleteLater();
        return;
    }

    auto bytes = reply->readAll();
    if ((bytes.size() < 7) || (bytes.mid(0,6).toStdString() == "<html>")){
        qCritical() << "Unable to read ADSB response" << reply->errorString();
        reply->deleteLater();
        return;
    }

    auto elementSize = (int)sizeof(struct binCraft);
    auto nbCraft = (bytes.count() / elementSize) - 1;

    struct binHeader hdr;
    memcpy((void*)&hdr, bytes.data(), sizeof(struct binHeader));

    qInfo() << nbCraft <<  " aircrafts fetched at " << QDateTime::currentDateTime().toString() ;

    QVector<struct binCraft> craftList(nbCraft);
    for(auto i = 0; i < nbCraft; i++){
        auto & craft = craftList[i];
        auto offset = (i * elementSize) + elementSize;
        memcpy((void*)&craft, bytes.data()+offset, elementSize);
    }
    getCraftModel()->refreshCraft(craftList);
    refreshScene();
    reply->deleteLater();
}

void MainWindow::refreshScene()
{
    const auto factor = 400.0;
    const auto size = 10;

    m_scene->clear();

    auto homex = getCraftModel()->getHome().longitude() * factor;
    auto homey = -getCraftModel()->getHome().latitude() * factor;
    QRectF homeRect(homex-(size/2), homey-(size/2), size, size);
    m_scene->addEllipse(homeRect, QPen(Qt::white));

    auto nbCircle = 4;
    auto circleFactor = (factor/nbCircle);
    for(auto i = 1; i <= nbCircle; i++){
        auto circleSize = i*circleFactor;
        QRectF sceneRect(homex-circleSize, homey-circleSize, 2*circleSize, 2*circleSize);
        m_scene->addEllipse(sceneRect, QPen(Qt::darkGreen));
        if(i==4){
            m_scene->setSceneRect(sceneRect);
        }
    }

    auto nbLines = 12;
    for(auto i = 0; i< nbLines; i++){
        QLineF angleline;
        angleline.setP1(QPointF(homex,homey));
        angleline.setAngle(i*30);
        angleline.setLength(factor);
        angleline.setP1(angleline.pointAt(0.9));
        m_scene->addLine(angleline, QPen(Qt::darkGray));
        auto ang = (((nbLines-i)*30)+90)%360;
        auto* text = m_scene->addText(QString::number(ang));
        text->setPos(angleline.center());
        text->setDefaultTextColor(Qt::darkGray);
    }

    for(auto& c: getCraftModel()->getCraft()){
        QColor color =  QColor(Qt::darkGreen);
        if(c.getSendAlert()){
            color = QColor(Qt::yellow);
        }else if(c.getSeen() > 120){
            color = QColor(0, 50, 10);
        }
        if(c.getDistanceToMe() < 100000){
            auto x = c.getPos().longitude() * factor;
            auto y = -c.getPos().latitude() * factor;

            QRectF aircraftRect(x-(size/2), y-(size/2), size, size);
            m_scene->addRect(aircraftRect, QPen(color), QBrush(color));

            auto* text = m_scene->addText(QString("%1\n%2 %3\n%4").arg(c.getCallsign()).arg((int)c.getAltitude()/100).arg((int)c.getGS()/10).arg(c.getTypeCode()));
            text->setPos(x+(4*size), y-(6*size));
            text->setDefaultTextColor(color);


            QLineF infoLine(QPointF(text->sceneBoundingRect().left(), text->sceneBoundingRect().center().y()), aircraftRect.topRight());
            m_scene->addLine(infoLine, QPen(color));

            if(c.getValidity(CraftValidity_true_heading_valid)){
                QLineF heading;
                heading.setP1(aircraftRect.center());
                heading.setAngle(90-c.getHeading());
                heading.setLength(size*4);
                m_scene->addLine(heading, QPen(color));
            }
        }
    }

    ui->graphicsView->fitInView( m_scene->sceneRect(), Qt::KeepAspectRatio);
}


