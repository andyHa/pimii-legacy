
#include <QtGui>

#include "mainwindow.h"

#include <iostream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
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

    engine = new PimiiWidget(this);

    setupFileMenu();
    setupRunMenu();
    setupHelpMenu();

    connect(engine,SIGNAL(log(QString)), this, SLOT(onLog(QString)));
    connect(engine,SIGNAL(status(EngineStatus)), this, SLOT(onReport(EngineStatus)));
    connect(engine,SIGNAL(computationStarted()), this, SLOT(onComputationStarted()));
    connect(engine,SIGNAL(computationStopped()), this, SLOT(onComputationStopped()));
    engine->getEngine()->println(QString("pimii v1.0 (c) 2011 Andreas Haufler"));
    QString path;
    char* pimiiHome = getenv("PIMII_HOME");
    if (pimiiHome != NULL) {
        path = QString(pimiiHome) + QDir::separator();
    } else {
       path = (QCoreApplication::applicationDirPath() + QDir::separator());
    }
    engine->getEngine()->addSourcePath(path);
    engine->getEngine()->println(QString("Library-Path: ")+path);

}

void MainWindow::onLog(const QString& str) {
    std::wcout << str.toStdWString() << std::endl;
    console->append(str);
}

void MainWindow::onComputationStarted() {
    status->setText(" RUNNING ");
}

void MainWindow::onComputationStopped() {
    status->setText(" STOPPED ");
}

void MainWindow::onReport(const EngineStatus& status) {
    statusBar()->showMessage(
                QString("INSTS: %1 (%9/ms), GCs: %2, Symbols: %3, Globals: %4, Cells: %5, Strings: %6, Numbers: %7, Decimals: %8")
                .arg(QString::number(status.instructionsExecuted),
                     QString::number(status.gcRuns),
                     QString::number(status.storageStats.numSymbols),
                     QString::number(status.storageStats.numGlobals),
                     QString("%1/%2").arg(QString::number(status.storageStats.cellsUsed),
                                          QString::number(status.storageStats.totalCells)),
                     QString("%1/%2").arg(QString::number(status.storageStats.stringsUsed),
                                          QString::number(status.storageStats.totalStrings)),
                     QString("%1/%2").arg(QString::number(status.storageStats.numbersUsed),
                                          QString::number(status.storageStats.totalNumbers)),
                     QString("%1/%2").arg(QString::number(status.storageStats.deicmalsUsed),
                                          QString::number(status.storageStats.totalDecimals)),
                     QString::number(status.instructionsExecuted / (status.timeElapsed + 1))
                     )
                );
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
    engine->evaluate(editor->document()->toPlainText(), currentFile);
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

