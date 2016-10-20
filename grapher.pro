#-------------------------------------------------
#
# Project created by QtCreator 2016-10-17T21:20:54
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = grapher
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    graph/node.cpp \
    graph/edge.cpp

HEADERS  += mainwindow.h \
    graph/node.h \
    graph/edge.h

FORMS    += mainwindow.ui
