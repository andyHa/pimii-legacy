#-------------------------------------------------
#
# Project created by QtCreator 2011-06-16T20:37:26
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = pimii
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    engine.cpp \
    bif.cpp \
    storage.cpp \
    compiler.cpp

HEADERS += \
    engine.h \
    lookuptable.h \
    valuetable.h \
    storage.h \
    env.h \
    tools.h \
    compiler.h

OTHER_FILES += \
    example.pi \
    kernel.pi \
    test.pi \
    playground.pi
