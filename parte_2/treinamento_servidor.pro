TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.c \
    ../utils/http_utils.c \
    ../utils/connection_item.c \
    ../utils/connection_manager.c \
    ../utils/test_suit.c \
    ../utils/thread_item.c \
    ../utils/file_utils.c

HEADERS += \
    ../utils/http_utils.h \
    ../utils/connection_item.h \
    ../utils/connection_manager.h \
    ../utils/test_suit.h \
    ../utils/thread_item.h \
    ../utils/file_utils.h

DISTFILES +=
