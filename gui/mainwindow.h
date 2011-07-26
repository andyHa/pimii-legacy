#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include <QElapsedTimer>

#include "highlighter.h"
#include "vm/env.h"
#include "vm/qengine.h"

QT_BEGIN_NAMESPACE
class QTextEdit;
QT_END_NAMESPACE

//! [0]
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);

    void println(String str);

public slots:
    void about();
    void newFile();
    void openFile(const QString &path = QString());
    void saveFile();
    void saveFileAs(const QString &path = QString());
    void runFile();

    void onLog(QString str);
    void onComputationStarted();
    void onComputationStopped();

private:
    void setupEditor();
    void setupFileMenu();
    void setupRunMenu();
    void setupHelpMenu();

    QString currentFile;
    QTextEdit *editor;
    QTextEdit* console;
    QSplitter *splitter;
    Highlighter *highlighter;
    QEngine* engine;
    QElapsedTimer timer;
};
//! [0]

#endif
