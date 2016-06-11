QT += core network
QT -= gui

CONFIG += c++11

TARGET = ICQ-server
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    server.cpp \
    user.cpp \
    room.cpp \
    connection.cpp

HEADERS += \
    server.h \
    user.h \
    room.h \
    connection.h
