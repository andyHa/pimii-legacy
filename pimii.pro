#-------------------------------------------------
#
# Project created by QtCreator 2011-06-16T20:37:26
#
#-------------------------------------------------

QT       += core gui

TARGET = pimii
TEMPLATE = app

SOURCES += main.cpp \
    vm/engine.cpp \
    vm/bif.cpp \
    vm/storage.cpp \
    compiler.cpp \
    gui/mainwindow.cpp \
    gui/highlighter.cpp \
    parser/tokenizer.cpp \
    vm/qengine.cpp # \
   # parser/parser.cpp

HEADERS += \
    vm/engine.h \
    vm/lookuptable.h \
    vm/valuetable.h \
    vm/storage.h \
    vm/env.h \
    tools.h \
    compiler.h \
    gui/mainwindow.h \
    gui/highlighter.h \
    parser/tokenizer.h \
    vm/qengine.h \
    vm/interceptor.h \
    parser/ast.h # \
 #   parser/parser.h

OTHER_FILES += \
    example.pi \
    kernel.pi \
    test.pi \
    playground.pi
