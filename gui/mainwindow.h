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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include <QElapsedTimer>
#include <QProgressBar>
#include <QLabel>

#include "vm/env.h"
#include "gui/pimiiwidget.h"
#include "gui/highlighter.h"

QT_BEGIN_NAMESPACE
class QTextEdit;
QT_END_NAMESPACE

//! [0]
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);

    void println(const QString& str);

public slots:
    void about();
    void newFile();
    void openFile(const QString& path = QString());
    void saveFile();
    void saveFileAs(const QString& path = QString());
    void runFile();

    void onLog(const QString& str);
    void onComputationStarted();
    void onComputationStopped();
    void onReport(const EngineStatus& status);

private:
    void setupEditor();
    void setupFileMenu();
    void setupRunMenu();
    void setupHelpMenu();

    QString currentFile;
    QTextEdit *editor;
    QTextEdit* console;
    QSplitter *splitter;
    QLabel* status;

    Highlighter *highlighter;
    PimiiWidget* engine;
};
//! [0]

#endif
