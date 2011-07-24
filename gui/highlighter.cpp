/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtGui>

#include <sstream>
#include <string>
#include <iostream>

#include "highlighter.h"
#include "parser/tokenizer.h"

Highlighter::Highlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    specialCharFormat.setForeground(Qt::darkBlue);
    specialCharFormat.setFontWeight(QFont::Bold);
    variableFormat.setForeground(Qt::darkMagenta);
    symbolFormat.setForeground(Qt::darkYellow);
    symbolFormat.setFontWeight(QFont::Bold);
    numberFormat.setForeground(Qt::darkGreen);
    stringFormat.setForeground(Qt::darkCyan);
    stringFormat.setFontItalic(true);
    colonCallFormat.setForeground(Qt::blue);
    colonCallFormat.setFontWeight(QFont::Bold);
    colonCallFormat.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
    colonCallFormat.setUnderlineColor(Qt::red);
}

void Highlighter::highlightBlock(const QString &text)
{
    std::wstringstream stream;
    stream << text.toStdWString();
    Tokenizer tokenizer(stream);
    InputToken t = tokenizer.fetch();
    while (t.type != TT_EOF) {
        switch(t.type) {
        case TT_NAME:
            if (t.tokenString[t.tokenString.length()-1] == ':') {
                setFormat(t.absolutePos, t.tokenString.length(), colonCallFormat);
            } else {
                setFormat(t.absolutePos, t.tokenString.length(), variableFormat);
            }
           break;
        case TT_SYMBOL:
           setFormat(t.absolutePos, t.tokenString.length() + 1, symbolFormat);
           break;
        case TT_STRING:
           setFormat(t.absolutePos, t.tokenString.length() + 2, stringFormat);
           break;
        case TT_NUMBER:
           setFormat(t.absolutePos, t.tokenString.length(), numberFormat);
           break;
        default:
           setFormat(t.absolutePos, t.tokenString.length(), specialCharFormat);
        }
        t = tokenizer.fetch();
    }
}
