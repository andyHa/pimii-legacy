
#include <QtGui>

#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    engine = new QEngine(this);
    setupFileMenu();
    setupRunMenu();
    setupHelpMenu();
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


    setCentralWidget(splitter);
    setWindowTitle(tr("Syntax Highlighter"));

    connect(engine,SIGNAL(log(QString)), this, SLOT(onLog(QString)));
    connect(engine,SIGNAL(computationStarted()), this, SLOT(onComputationStarted()));
    connect(engine,SIGNAL(computationStopped()), this, SLOT(onComputationStopped()));
    engine->getEngine().println(String(L"pimii v1.0 (c) 2011 Andreas Haufler"));
    std::string path;
    char* pimiiHome = getenv("PIMII_HOME");
    if (pimiiHome != NULL) {
        path = std::string(pimiiHome) + QDir::separator().toAscii();
    } else {
       path = (QCoreApplication::applicationDirPath() + QDir::separator()).toStdString();
    }
    engine->getEngine().addSourcePath(asString(path));
    engine->getEngine().println(String(L"Library-Path: ")+asString(path));

}

void MainWindow::onLog(QString str) {
    console->append(str);
}

void MainWindow::onComputationStarted() {
    timer.start();
}

void MainWindow::onComputationStopped() {
    console->append(QString("Eval took: %1 ms").arg(QString::number(timer.elapsed())));
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
            file.write(editor->document()->toPlainText().toUtf8());
        }
    } else {
        saveFileAs();
    }
}

void MainWindow::saveFileAs(const QString &path) {
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), "", "pimii Files (*.pi)");
    if (!fileName.isEmpty()) {
        currentFile = fileName;
        QFile file(currentFile);
        if (file.open(QFile::ReadWrite | QFile::Text)) {
            file.write(editor->document()->toPlainText().toUtf8());
        }
    }
}

void MainWindow::runFile() {
    engine->evaluate(editor->document()->toPlainText(),QString("test"));
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

