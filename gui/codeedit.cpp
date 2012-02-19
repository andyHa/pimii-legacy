#include "codeedit.h"
#include <QKeyEvent>


CodeEdit::CodeEdit(QWidget *parent) : QPlainTextEdit(parent) {

}

CodeEdit::~CodeEdit() {

}

void CodeEdit::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Tab) {
        event = new QKeyEvent(QEvent::KeyPress,
                              Qt::Key_Tab,
                              Qt::NoModifier,
                              "   ");
    }
    QPlainTextEdit::keyPressEvent(event);
}

