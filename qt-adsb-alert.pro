QT       += core gui network positioning qml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++14 console

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    craftmodel.cpp \
    craftproxymodel.cpp \
    main.cpp \
    mainwindow.cpp \
    networknotifier.cpp \
    zstddeclib.c \
    zstdframe.cpp


HEADERS += \
    adsb.h \
    craftmodel.h \
    craftproxymodel.h \
    mainwindow.h \
    networknotifier.h \
    zstd.h \
    zstdframe.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    icao_aircraft_types2.js \
    sendAlert.mjs \
    setup.ini

RESOURCES += \
    resource.qrc


