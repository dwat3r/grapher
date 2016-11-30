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
        graphics.cpp

HEADERS  += mainwindow.h \
         graphics.h

FORMS    += mainwindow.ui


!win32 {
  QMAKE_CXXFLAGS += -std=c++11
  QMAKE_CXXFLAGS_WARN_ON += -Wno-inconsistent-missing-override
}

win32 {
  CONFIG += c++11
}
