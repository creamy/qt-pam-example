#-------------------------------------------------
#
# Project created by QtCreator 2012-06-30T09:26:56
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = pkg_gui
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    logindialog.cpp

HEADERS  += mainwindow.h \
    logindialog.h

FORMS    += mainwindow.ui \
    logindialog.ui

LIBS     += -lpam
