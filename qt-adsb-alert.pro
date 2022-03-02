QT       += core gui network positioning

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++14

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Smtp/emailaddress.cpp \
    Smtp/mimeattachment.cpp \
    Smtp/mimecontentformatter.cpp \
    Smtp/mimefile.cpp \
    Smtp/mimehtml.cpp \
    Smtp/mimeinlinefile.cpp \
    Smtp/mimemessage.cpp \
    Smtp/mimemultipart.cpp \
    Smtp/mimepart.cpp \
    Smtp/mimetext.cpp \
    Smtp/quotedprintable.cpp \
    Smtp/smtpclient.cpp \
    craftmodel.cpp \
    craftproxymodel.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    Smtp/SmtpMime \
    Smtp/emailaddress.h \
    Smtp/mimeattachment.h \
    Smtp/mimecontentformatter.h \
    Smtp/mimefile.h \
    Smtp/mimehtml.h \
    Smtp/mimeinlinefile.h \
    Smtp/mimemessage.h \
    Smtp/mimemultipart.h \
    Smtp/mimepart.h \
    Smtp/mimetext.h \
    Smtp/quotedprintable.h \
    Smtp/smtpclient.h \
    Smtp/smtpexports.h \
    adsb.h \
    craftmodel.h \
    craftproxymodel.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    icao_aircraft_types2.js \
    setup.ini

RESOURCES += \
    resource.qrc


