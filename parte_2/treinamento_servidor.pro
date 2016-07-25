TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.c \
    ../utils/http_utils.c

HEADERS += \
    ../utils/http_utils.h
