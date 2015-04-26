TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += c++11

SOURCES += main.cpp \
    ram.cpp \
    mmu.cpp \
    processqueue.cpp \
    mprocess.cpp \
    util/rand.cpp \
    my_global.cpp

include(deployment.pri)
qtcAddDeployment()

HEADERS += \
    ram.h \
    mmu.h \
    my_global.h \
    mprocess.h \
    processqueue.h \
    util/rand.h \
    util/range.h

#DEFINES += VISUALIZE
