#-------------------------------------------------
#
# Project created by QtCreator 2019-10-24T11:47:43
#
#-------------------------------------------------

#not qt
QT       += xml

TARGET = libnetservice

TEMPLATE = lib

CONFIG += c++11
#CONFIG += staticlib

DEFINES += LIB_NETSERVICE

DESTDIR = $$PWD/../../bin

LIBDIR += $$PWD/../../bin

CONFIG(debug, debug|release) {
    win32:LIBS += -lws2_32 \
                  -L$${LIBDIR} -llibutils \
                  -L$${LIBDIR} -llibcommsocket \
                  -L$${LIBDIR} -llibprotocol \
                  -L$${LIBDIR} -llibscommunicator
}
else {
    win32:LIBS += -lws2_32 \
                  -L$${LIBDIR} -llibutils \
                  -L$${LIBDIR} -llibcommsocket \
                  -L$${LIBDIR} -llibprotocol \
                  -L$${LIBDIR} -llibscommunicator
}


SOURCES += \
    netservice/core/service.cpp \
    netservice.cpp


HEADERS += \
    netservice.h \
    netservice/netservice_interface.h \
    netservice/core/service.h \
    netservice/common/netservice_def.h

