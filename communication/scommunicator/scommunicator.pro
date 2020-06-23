#-------------------------------------------------
#
# Project created by QtCreator 2019-10-24T11:47:43
#
#-------------------------------------------------

#not qt
QT       += xml

TARGET = libscommunicator

TEMPLATE = lib

CONFIG += c++11
#CONFIG += staticlib

DEFINES += LIB_SCOMMUNICATOR

DESTDIR = $$PWD/../../bin

LIBDIR += $$PWD/../../bin

CONFIG(debug, debug|release) {
    win32:LIBS += -lws2_32 \
                  -L$${LIBDIR} -llibutils \
                  -L$${LIBDIR} -llibcommsocket \
                  -L$${LIBDIR} -llibprotocol
}
else {
    win32:LIBS += -lws2_32 \
                  -L$${LIBDIR} -llibutils \
                  -L$${LIBDIR} -llibcommsocket \
                  -L$${LIBDIR} -llibprotocol
}


SOURCES += \
    scommunicator/manager/scommmanager.cpp \
    scommunicator/service/scommservice.cpp \
    scommunicator/pubfunc/pubfunc.cpp \
    scommunicator.cpp

HEADERS += \
    scommunicator.h \
    scommunicator/common/scommunicator_def.h \
    scommunicator/scommunicator_interface.h \
    scommunicator/manager/scommmanager.h \
    scommunicator/service/scommservice.h \
    scommunicator/pubfunc/pubfunc.h

