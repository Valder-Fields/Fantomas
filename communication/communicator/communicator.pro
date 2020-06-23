#-------------------------------------------------
#
# Project created by QtCreator 2019-10-24T11:47:43
#
#-------------------------------------------------

#not qt
QT       += xml

TARGET = libcommunicator

TEMPLATE = lib

CONFIG += c++11
#CONFIG += staticlib

DEFINES += LIB_COMMUNICATOR

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
    communicator/manager/commmanager.cpp \
    communicator/service/commservice.cpp \
    communicator/pubfunc/pubfunc.cpp \
    communicator.cpp

HEADERS += \
    communicator.h \
    communicator/common/communicator_def.h \
    communicator/communicator_interface.h \
    communicator/manager/commmanager.h \
    communicator/service/commservice.h \
    communicator/pubfunc/pubfunc.h
