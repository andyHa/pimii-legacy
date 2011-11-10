#-------------------------------------------------
#
# Project created by QtCreator 2011-06-16T20:37:26
#
#-------------------------------------------------

QT       += core gui webkit

QMAKE_CXXFLAGS += -Wall

TARGET = pimii
TEMPLATE = app

SOURCES += main.cpp \
    vm/engine.cpp \
    vm/storage.cpp \
    compiler/tokenizer.cpp \
    compiler/compiler.cpp \
    gui/mainwindow.cpp \
    gui/highlighter.cpp \
    gui/webwindow.cpp \
    bif/coreextension.cpp \
    bif/filesextension.cpp \
    bif/webextension.cpp \
    tools/logger.cpp \
    gui/editorwindow.cpp

HEADERS += \
    vm/engine.h \
    vm/lookuptable.h \
    vm/valuetable.h \
    vm/storage.h \
    vm/env.h \
    compiler/tokenizer.h \
    compiler/compiler.h \
    gui/mainwindow.h \
    gui/highlighter.h \
    gui/webwindow.h \
    vm/reference.h \
    bif/coreextension.h \
    bif/filesextension.h \
    bif/engineextension.h \
    bif/callcontext.h \
    bif/webextension.h \
    tools/logger.h \
    tools/average.h \
    gui/editorwindow.h

OTHER_FILES += \
    example.pi \
    kernel.pi \
    test.pi \
    playground.pi \
    lib/pimii.pi \
    lib/files.pi \
    start.pi \
    examples/test.pi \
    examples/playground.pi \
    examples/kernel.pi \
    examples/example.pi \
    lib/datetime.pi \
    performance-tests.txt \
    examples/performanceTest.pi

FORMS += \
    gui/webwindow.ui \
    gui/editorwindow.ui

RESOURCES += \
    gui/resources.qrc














