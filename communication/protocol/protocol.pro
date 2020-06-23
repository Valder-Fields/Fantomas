#-------------------------------------------------
#
# Project created by QtCreator 2019-10-24T11:47:43
#
#-------------------------------------------------

#通讯模块主要是供Qt界面程序使用,故协议添加了Qt模块(QDebug)以便在IDE打印调试信息
#如不需要打印调试信息,可去掉Qt模块
QT       -= core gui

TARGET = libprotocol

TEMPLATE = lib

DEFINES += LIB_PROTOCOL

DESTDIR = $$PWD/../../bin

LIBDIR +=

CONFIG(debug, debug|release) {
#    win32:LIBS += -lws2_32 \
#                  -L$${LIBDIR} -lrwsocket
}
else {
#    win32:LIBS += -lws2_32 \
#                  -L$${LIBDIR} -lrwsocket
}

SOURCES += \
    protocol/base/baseprotocol.cpp \
    protocol/simpletlv/protocol_simpletlv.cpp


HEADERS += \
    protocol.h \
    protocol/protocol_interface.h \
    protocol/common/protocol_def.h \
    protocol/base/baseprotocol.h \
    protocol/simpletlv/protocol_simpletlv.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
