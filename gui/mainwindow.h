#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include <QElapsedTimer>
#include <QProgressBar>
#include <QLabel>

#include "vm/env.h"
#include "gui/qengine.h"
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
    void onReport(EngineStatus status);

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
    QEngine* engine;
};
//! [0]

#endif
