#-------------------------------------------------
#
# Project created by QtCreator 2016-08-16T16:45:29
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GuiServer
TEMPLATE = app


SOURCES += main.cpp\
    ../utils/Config.cpp \
    ../Viewer/form.cpp

HEADERS  += \
    ../utils/Config.h \
    ../Viewer/form.h

FORMS    += \
    ../Viewer/form.ui
