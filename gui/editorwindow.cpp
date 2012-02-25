#include "editorwindow.h"
#include "ui_editorwindow.h"

#include <QFileDialog>
#include <QTextLayout>
#include <QScrollBar>
#include <QClipboard>

EditorWindow::EditorWindow(Engine* engine, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::EditorWindow),
    engine(engine)
{
    ui->setupUi(this);
    status = new QLabel();
    status->setText(" STOPPED ");
    ui->statusbar->addPermanentWidget(status);
    currentFile = QString("unnamed");
    highlighter = new Highlighter(ui->editor->document());
    Logger::addAppender(this);

    connect(engine,SIGNAL(onEngineStarted()), this, SLOT(onEngineStarted()));
    connect(engine,SIGNAL(onEngineStopped()), this, SLOT(onEngineStopped()));
    connect(engine,SIGNAL(onEnginePanic(Atom, Word, QString, QString)),
            this, SLOT(onEnginePanic(Atom,Word,QString,QString)));
}

void EditorWindow::append(const QString& msg, const QString& pos) {
    ui->console->insertPlainText(msg);
    ui->console->insertPlainText("\n");
    ui->console->verticalScrollBar()->setSliderPosition(
    ui->console->verticalScrollBar()->maximum());
}

void EditorWindow::onEngineStarted() {
    status->setText(" RUNNING ");
}

void EditorWindow::onEngineStopped() {
    status->setText(" STOPPED ");
}

void EditorWindow::onEnginePanic(Atom file,
                                 Word line,
                                 const QString &error,
                                 const QString &status) {
    ui->console->insertPlainText(error);
    ui->console->insertPlainText("\n");
    ui->console->insertPlainText(status);
    ui->console->verticalScrollBar()->setSliderPosition(
    ui->console->verticalScrollBar()->maximum());
}

EditorWindow::~EditorWindow()
{
    Logger::removeAppender(this);
    delete ui;
}

void EditorWindow::on_actionClear_triggered()
{
    ui->console->clear();
}

void EditorWindow::on_action_New_triggered()
{
    ui->editor->clear();
    currentFile = QString("unnamed");
}

void EditorWindow::on_action_Run_File_triggered()
{
    ui->console->clear();
    engine->eval(ui->editor->document()->toPlainText(), currentFile, false);
}

void EditorWindow::on_action_Inspect_Selection_triggered()
{
    if (ui->editor->textCursor().hasSelection()) {
        engine->eval(ui->editor->textCursor().selectedText(),
                     currentFile,
                     true);
    } else {
        engine->eval(ui->editor->document()->toPlainText(), currentFile, true);
    }
}

void EditorWindow::on_action_Quit_triggered()
{
    qApp->quit();
}

void EditorWindow::on_action_Save_triggered()
{
    if (!currentFile.isEmpty()) {
        QFile file(currentFile);
        if (file.open(QFile::ReadWrite | QFile::Text | QFile::Truncate)) {
            file.write(ui->editor->document()->toPlainText().toUtf8());
        }
    } else {
        on_actionSave_as_triggered();
    }
}

void EditorWindow::on_actionSave_as_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save File"),
                                                    "",
                                                    "pimii Files (*.pi)");
    if (!fileName.isEmpty()) {
        currentFile = fileName;
        QFile file(currentFile);
        if (file.open(QFile::ReadWrite | QFile::Text | QFile::Truncate)) {
            file.write(ui->editor->document()->toPlainText().toUtf8());
        }
    }
}

void EditorWindow::on_action_Open_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open File"),
                                                    "",
                                                    "pimii Files (*.pi)");
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QFile::ReadOnly | QFile::Text)) {
            ui->editor->setPlainText(file.readAll().replace("\t", "   "));
        }
        currentFile = fileName;
    }
}

QString replaceLatexChars(QString input) {
    return input.replace("{","\\{").replace("}","\\}").replace("#","\\#").replace("&","\\&");
}

void EditorWindow::on_actionLaTex_triggered()
{
    ui->console->clear();
    QRegExp spaces(" \\d{2,}");
    QString output = "{\\ttfamily\n";
    QTextBlock b = ui->editor->document()->begin();
    int line = 1;
    while(b.length() > 0) {
        int lastEnd = 0;
        output += "\\verb+" + QString("%1").arg(line++, 2, 10) + "   +";
        foreach(QTextLayout::FormatRange r, b.layout()->additionalFormats()) {
            if (r.start != lastEnd) {
                QString line = b.text().mid(lastEnd, r.start - lastEnd);
                if (spaces.exactMatch(line)) {
                    output += "\\verb+"+line+"+";
                } else {
                    output += replaceLatexChars(line);
                }
            }
            if (r.format.fontWeight() == QFont::Bold) {
                output += "\\textbf{" + replaceLatexChars(b.text().mid(r.start, r.length))+"}";
            } else if (r.format.fontItalic()){
                output += "\\textit{" + replaceLatexChars(b.text().mid(r.start, r.length))+"}";
            } else {
                output += replaceLatexChars(b.text().mid(r.start, r.length));
            }
            lastEnd = r.start + r.length;
        }
        b = b.next();

        output +=" \\\\\n";
    }
    output += "}";
    append(output,"");
    QApplication::clipboard()->setText(output);
}
