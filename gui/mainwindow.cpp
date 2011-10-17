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

#include "mainwindow.h"

#include <iostream>

MainWindow::MainWindow(Engine* engine, QWidget *parent)
    : QMainWindow(parent), engine(engine)
{

    setupEditor();
    splitter = new QSplitter(Qt::Vertical, parent);
    splitter->addWidget(editor);
    console = new QTextEdit();
    console->setMinimumHeight(100);
    console->setReadOnly(true);
    console->setFont(QFont("Courier", 12));
    QPalette p=palette();
    p.setColor(QPalette::Base, QColor (Qt::black));
    p.setColor(QPalette::Text, Qt::white);
    console->setPalette(p);
    splitter->addWidget(console);
    currentFile = QString("unnamed");
    status = new QLabel();
    status->setText(" STOPPED ");
    statusBar()->addPermanentWidget(status);

    setCentralWidget(splitter);
    setWindowTitle(tr("pimii 1.0"));
    qRegisterMetaType<EngineStatus>("EngineStatus");

    setupFileMenu();
    setupRunMenu();
    setupHelpMenu();

    connect(engine,SIGNAL(onLog(QString)), this, SLOT(onLog(QString)));
    connect(engine,SIGNAL(onEngineStarted()), this, SLOT(onEngineStarted()));
    connect(engine,SIGNAL(onEngineStopped()), this, SLOT(onEngineStopped()));
    connect(engine,SIGNAL(onEnginePanic(Atom, Word, QString, QString)),
            this, SLOT(onEnginePanic(Atom,Word,QString,QString)));
    engine->println(QString("pimii v1.0 (c) 2011 Andreas Haufler"));
    QString path;
    char* pimiiHome = getenv("PIMII_HOME");
    if (pimiiHome != NULL) {
        path = QString(pimiiHome) + QDir::separator();
    } else {
       path = (QCoreApplication::applicationDirPath() + QDir::separator());
    }
    engine->addSourcePath(path);
    engine->println(QString("Library-Path: ")+path);

}

void MainWindow::onLog(const QString& str) {
    std::wcout << str.toStdWString() << std::endl;
    console->append(str);
}

void MainWindow::onEngineStarted() {
    status->setText(" RUNNING ");
}

void MainWindow::onEngineStopped() {
    status->setText(" STOPPED ");
}

void MainWindow::onEnginePanic(Atom file, Word line, const QString &error, const QString &status) {
    console->append(error);
    console->append(status);
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About Syntax Highlighter"),
                tr("<p>The <b>Syntax Highlighter</b> example shows how " \
                   "to perform simple syntax highlighting by subclassing " \
                   "the QSyntaxHighlighter class and describing " \
                   "highlighting rules using regular expressions.</p>"));
}

void MainWindow::newFile()
{
    editor->clear();
    currentFile = QString("");
}

void MainWindow::openFile(const QString &path)
{
    QString fileName = path;

    if (fileName.isNull())
        fileName = QFileDialog::getOpenFileName(this,
            tr("Open File"), "", "pimii Files (*.pi)");

    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QFile::ReadOnly | QFile::Text)) {
            editor->setPlainText(file.readAll());
        }
        currentFile = fileName;
    }
}

void MainWindow::saveFile() {
    if (!currentFile.isEmpty()) {
        QFile file(currentFile);
        if (file.open(QFile::ReadWrite | QFile::Text)) {
            file.write(editor->toPlainText().toUtf8());
        }
    } else {
        saveFileAs();
    }
}

void MainWindow::saveFileAs(const QString &path) {
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), path, "pimii Files (*.pi)");
    if (!fileName.isEmpty()) {
        currentFile = fileName;
        QFile file(currentFile);
        if (file.open(QFile::ReadWrite | QFile::Text)) {
            file.write(editor->document()->toPlainText().toUtf8());
        }
    }
}

void MainWindow::runFile() {
    console->clear();
    engine->eval(editor->document()->toPlainText(), currentFile);
}

void MainWindow::setupEditor()
{
    QFont font;
    font.setFamily("Courier");
    font.setFixedPitch(true);
    font.setPointSize(10);

    editor = new QTextEdit;
    editor->setFont(font);

    highlighter = new Highlighter(editor->document());
}

void MainWindow::setupFileMenu()
{
    QMenu *fileMenu = new QMenu(tr("&File"), this);
    menuBar()->addMenu(fileMenu);

    fileMenu->addAction(tr("&New"), this, SLOT(newFile()),
                        QKeySequence::New);

    fileMenu->addAction(tr("&Open..."), this, SLOT(openFile()),
                        QKeySequence::Open);

    fileMenu->addAction(tr("&Save"), this, SLOT(saveFile()),
                        QKeySequence::Save);

    fileMenu->addAction(tr("Save &As"), this, SLOT(saveFileAs()),
                        QKeySequence::SaveAs);

    fileMenu->addAction(tr("E&xit"), qApp, SLOT(quit()),
                        QKeySequence::Quit);
}
void MainWindow::setupRunMenu()
{
    QMenu *runMenu = new QMenu(tr("&Run"), this);
    menuBar()->addMenu(runMenu);

    runMenu->addAction(tr("&Execute"), this, SLOT(runFile()), QKeySequence(Qt::Key_F5));
    runMenu->addAction(tr("&Interrupt"), engine, SLOT(interrupt()), QKeySequence(Qt::Key_F6));
    runMenu->addAction(tr("Con&tinue"), engine, SLOT(continueEvaluation()), QKeySequence(Qt::Key_F7));
}

void MainWindow::setupHelpMenu()
{
    QMenu *helpMenu = new QMenu(tr("&Help"), this);
    menuBar()->addMenu(helpMenu);

    helpMenu->addAction(tr("&About"), this, SLOT(about()));
    helpMenu->addAction(tr("About &Qt"), qApp, SLOT(aboutQt()));
}

