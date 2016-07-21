TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.c \
    http_utils.c

DISTFILES +=

HEADERS += \
    socket.h \
    http_utils.h
