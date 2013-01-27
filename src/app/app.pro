#-------------------------------------------------
#
# Project created by QtCreator 2012-12-08T00:32:05
#
#-------------------------------------------------

QT       += core gui

TARGET = finFoil
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

INCLUDEPATH += ../patheditor/ \
            ../fineditors/contoureditor/

DEPENDPATH += ..

include(../patheditor/patheditor.pri)
include(../fineditors/contoureditor/contoureditor.pri)

OTHER_FILES += ../../LICENSE.BSD \
            ../../README