/**
    Licensed to the Apache Software Foundation (ASF) under one
    or more contributor license agreements.  See the NOTICE file
    distributed with this work for additional information
    regarding copyright ownership.  The ASF licenses this file
    to you under the Apache License, Version 2.0 (the
    "License"); you may not use this file except in compliance
    with the License.  You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing,
    software distributed under the License is distributed on an
    "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
    KIND, either express or implied.  See the License for the
    specific language governing permissions and limitations
    under the License.
 */

#include <QtGui>

#include <sstream>
#include <string>
#include <iostream>

#include "gui/highlighter.h"
#include "compiler/tokenizer.h"

Highlighter::Highlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    specialCharFormat.setForeground(Qt::darkBlue);
    specialCharFormat.setFontWeight(QFont::Bold);
    variableFormat.setForeground(Qt::darkMagenta);
    symbolFormat.setForeground(Qt::darkYellow);
    symbolFormat.setFontWeight(QFont::Bold);
    numberFormat.setForeground(Qt::darkGreen);
    decimalFormat.setForeground(Qt::darkGreen);
    decimalFormat.setFontItalic(true);
    stringFormat.setForeground(Qt::darkCyan);
    stringFormat.setFontItalic(true);
    commentFormat.setForeground(Qt::darkRed);
    commentFormat.setFontItalic(true);
    unknownFormat.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
    unknownFormat.setUnderlineColor(Qt::red);
    colonCallFormat.setForeground(Qt::blue);
    colonCallFormat.setFontWeight(QFont::Bold);
    xmlFormat.setForeground(Qt::gray);
    xmlFormat.setFontWeight(QFont::Bold);
}

void Highlighter::highlightBlock(const QString &text)
{
    Tokenizer tokenizer(text, false);
    InputToken t = tokenizer.fetch();
    while (t.type != TT_EOF) {
        switch(t.type) {
        case TT_NAME:
            if (tokenizer.getString(t).endsWith(":")) {
                setFormat(t.absolutePos, t.length, colonCallFormat);
            } else {
                setFormat(t.absolutePos, t.length, variableFormat);
            }
           break;
        case TT_SYMBOL:
           setFormat(t.absolutePos, t.length, symbolFormat);
           break;
        case TT_STRING:
           setFormat(t.absolutePos, t.length, stringFormat);
           break;
        case TT_COMMENT:
           setFormat(t.absolutePos, t.length, commentFormat);
           break;
        case TT_UNKNOWN:
           setFormat(t.absolutePos, t.length, unknownFormat);
           break;
        case TT_NUMBER:
           setFormat(t.absolutePos, t.length, numberFormat);
           break;
        case TT_DECIMAL:
           setFormat(t.absolutePos, t.length, decimalFormat);
           break;
        default:
           setFormat(t.absolutePos, t.length, specialCharFormat);
        }
        t = tokenizer.fetch();
    }
}
