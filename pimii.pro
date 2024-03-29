#-------------------------------------------------
#
# Project created by QtCreator 2011-06-16T20:37:26
#
#-------------------------------------------------

QT       += core gui

QMAKE_CXXFLAGS += -Wall

TARGET = pimii
TEMPLATE = app

SOURCES += main.cpp \
    vm/engine.cpp \
    vm/storage.cpp \
    compiler/tokenizer.cpp \
    compiler/compiler.cpp \
    gui/highlighter.cpp \
    bif/coreextension.cpp \
    bif/filesextension.cpp \
    tools/logger.cpp \
    gui/editorwindow.cpp \
    gui/codeedit.cpp

HEADERS += \
    vm/engine.h \
    vm/lookuptable.h \
    vm/valuetable.h \
    vm/storage.h \
    vm/env.h \
    compiler/tokenizer.h \
    compiler/compiler.h \
    gui/highlighter.h \
    vm/reference.h \
    bif/coreextension.h \
    bif/filesextension.h \
    bif/engineextension.h \
    bif/callcontext.h \
    tools/logger.h \
    tools/average.h \
    gui/editorwindow.h \
    vm/array.h \
    gui/codeedit.h

OTHER_FILES += \
    example.pi \
    kernel.pi \
    test.pi \
    playground.pi \
    lib/pimii.pi \
    start.pi \
    examples/example.pi \
    performance-tests.txt \
    examples/performanceTest.pi \
    README.md \
    deploy/start.pi \
    deploy/lib/pimii.pi \
    deploy/examples/performanceTest.pi \
    deploy/examples/example.pi

FORMS += \
    gui/editorwindow.ui

RESOURCES += \
    gui/resources.qrc

ICON = gui/pimii.icns

































