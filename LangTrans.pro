#-------------------------------------------------
#
# Project created by QtCreator 2015-02-13T14:41:45
#
#-------------------------------------------------

QT       += core

QT       -= gui

include(3rdparty/qtxlsx/src/xlsx/qtxlsx.pri)

TARGET = LangTrans
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    translator.cpp \
    transmessage.cpp \
    translanguage.cpp

HEADERS += \
    globaltype.h \
    translator.h \
    transmessage.h \
    translanguage.h
