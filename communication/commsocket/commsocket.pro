#-------------------------------------------------
#
# Project created by QtCreator 2019-10-24T11:47:43
#
#-------------------------------------------------

#not qt
QT       -= core gui

TARGET = libcommsocket
TEMPLATE = lib

DEFINES += LIB_COMMSOCKET INDEPENDENT_MODULE_COMM_COMMSOCKET

#DEFINES += QT_DEPRECATED_WARNINGS

#输出目录:系统总输出路径
DESTDIR = $$PWD/../../bin
LIBDIR  = $$PWD/../../bin

CONFIG(debug, debug|release) {
    win32:LIBS += -lws2_32 \
                  -L$${LIBDIR} -llibutils
} else {
    win32:LIBS += -lws2_32 \
                  -L$${LIBDIR} -llibutils
}

SOURCES += \
    commsocket/udp/comm_udp.cpp \
    commsocket/base/comm_base.cpp \
    commsocket/tcp/comm_tcp.cpp

HEADERS += \
        commsocket.h \
    commsocket/comm_interface.h \
    commsocket/udp/comm_udp.h \
    commsocket/base/comm_base.h \
    commsocket/tcp/comm_tcp.h \
    commsocket/base/commsocket_def.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
