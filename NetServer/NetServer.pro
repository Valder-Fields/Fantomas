#-------------------------------------------------
#
# Project created by QtCreator 2019-04-04T11:49:52
#
#-------------------------------------------------

QT       += core gui xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = NetServer
TEMPLATE = app

CONFIG += console

DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += c++11

INCLUDEPATH += $$PWD/third/communication

DESTDIR = $$PWD/../bin

LIBDIR += $$PWD/../bin

CONFIG(debug, debug|release) {
    win32:LIBS += -lws2_32 \
                  -L$${LIBDIR} -llibutils \
                  -L$${LIBDIR} -llibcommsocket \
                  -L$${LIBDIR} -llibprotocol \
                  -L$${LIBDIR} -llibscommunicator \
                  -L$${LIBDIR} -llibnetservice
}
else {
    win32:LIBS += -lws2_32 \
                  -L$${LIBDIR} -llibutils \
                  -L$${LIBDIR} -llibcommsocket \
                  -L$${LIBDIR} -llibprotocol \
                  -L$${LIBDIR} -llibscommunicator \
                  -L$${LIBDIR} -llibnetservice
}

SOURCES += \
        main.cpp \
        mainwindow.cpp \
        fnetservice.cpp

HEADERS += \
        mainwindow.h \
        fnetservice.h

FORMS += \
        mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
