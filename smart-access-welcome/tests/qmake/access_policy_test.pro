QT -= gui
CONFIG += console testcase c++17
TEMPLATE = app
TARGET = access_policy_test
SOURCES += ../access_policy_test.cpp ../../modules/access-control/access_policy.cpp
INCLUDEPATH += ../../modules/access-control/include
DESTDIR = bin
