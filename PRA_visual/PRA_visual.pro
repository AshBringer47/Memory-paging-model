#-------------------------------------------------
#
# Project created by QtCreator 2015-02-19T13:28:17
#
#-------------------------------------------------

QT       += core gui
CONFIG += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PRA_visualization
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    ../PRA_model/mmu.cpp \
    ../PRA_model/mprocess.cpp \
    ../PRA_model/processqueue.cpp \
    ../PRA_model/ram.cpp \
    ../PRA_model/util/rand.cpp

HEADERS  += mainwindow.h \
    ../PRA_model/mmu.h \
    ../PRA_model/mprocess.h \
    ../PRA_model/my_global.h \
    ../PRA_model/processqueue.h \
    ../PRA_model/ram.h \
    ../PRA_model/util/rand.h

FORMS    += mainwindow.ui

DEFINES += VISUALIZE
