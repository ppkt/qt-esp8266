#-------------------------------------------------
#
# Project created by QtCreator 2014-11-19T18:54:15
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets serialport network

TARGET = Wifi
TEMPLATE = app


SOURCES += main.cpp\
        wifi.cpp \
    accesspoint.cpp

HEADERS  += wifi.h \
    accesspoint.h

FORMS    += wifi.ui
