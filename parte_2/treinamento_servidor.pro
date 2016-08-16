TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXFLAGS += -std=c++0x -pthread
LIBS += -pthread

SOURCES += main.c \
    ../utils/http_utils.c \
    ../utils/connection_item.c \
    ../utils/connection_manager.c \
    ../utils/test_suit.c \
    ../utils/file_utils.c \
    ../utils/request_list.c \
    ../utils/request_manager.c \
    ../utils/thread.c \
    ../Viewer/main_form.cpp

HEADERS += \
    ../utils/http_utils.h \
    ../utils/connection_item.h \
    ../utils/connection_manager.h \
    ../utils/test_suit.h \
    ../utils/file_utils.h \
    ../utils/request_list.h \
    ../utils/request_manager.h \
    ../utils/thread.h \
    ../Viewer/main_form.h

DISTFILES += \
    ../build_server/script_test.py \
    ../build_server/script_test_with_put.py \
    ../build_server/put_get_test.py

FORMS += \
    ../Viewer/main_form.ui
