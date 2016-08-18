TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
##QT     += widgets

QMAKE_CXXFLAGS += -std=c++11
QMAKE_CXXFLAGS += -pthread
LIBS += -pthread

SOURCES += \
    ../utils/http_utils.c \
    ../utils/connection_item.c \
    ../utils/connection_manager.c \
    ../utils/test_suit.c \
    ../utils/file_utils.c \
    ../utils/request_list.c \
    ../utils/request_manager.c \
    ../utils/thread.c \
    ../utils/Config.cpp \
    main.c \
    ../utils/handle_settings.c

HEADERS += \
    ../utils/http_utils.h \
    ../utils/connection_item.h \
    ../utils/connection_manager.h \
    ../utils/test_suit.h \
    ../utils/file_utils.h \
    ../utils/request_list.h \
    ../utils/request_manager.h \
    ../utils/thread.h \
    ../utils/Config.h \
    ../utils/handle_settings.h

DISTFILES += \
    ../build_server/script_test.py \
    ../build_server/script_test_with_put.py \
    ../build_server/put_get_test.py

FORMS +=
