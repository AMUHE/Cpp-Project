QT += core network sql testlib
QT -= gui
CONFIG += console testcase c++17
TEMPLATE = app
TARGET = infrastructure_test
SOURCES += ../infrastructure_test.cpp \
    ../../modules/config/app_config.cpp \
    ../../modules/persistence/access_event_store.cpp \
    ../../modules/device/door_controller.cpp
HEADERS += ../../modules/device/include/saw/device/door_controller.h
INCLUDEPATH += ../../modules/config/include ../../modules/persistence/include ../../modules/device/include
DESTDIR = bin
