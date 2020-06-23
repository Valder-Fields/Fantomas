#-------------------------------------------------
#
# Project created by QtCreator 2019-10-24T11:47:43
#
#-------------------------------------------------

#not qt
QT       += core

TARGET = libutils

TEMPLATE = lib

CONFIG += c++11
#CONFIG += staticlib

DEFINES += LIB_UTILS

DESTDIR = $$PWD/../bin

LIBDIR += $$PWD/../bin

CONFIG(debug, debug|release) {

}
else {

}


SOURCES += \
    utils/func/utilfunc.cpp


HEADERS += \
    utils.h \
    utils/utils_interface.h \
    utils/common/utils_def.h \
    utils/func/utilfunc.h


INCLUDEPATH += $$PWD/.
