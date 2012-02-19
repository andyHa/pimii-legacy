#ifndef EDITORWINDOW_H
#define EDITORWINDOW_H

#include <QMainWindow>
#include <QLabel>

#include "gui/highlighter.h"
#include "tools/logger.h"
#include "vm/engine.h"


namespace Ui {
    class EditorWindow;
}

class EditorWindow : public QMainWindow, Appender
{
    Q_OBJECT

public:
    explicit EditorWindow(Engine* engine, QWidget* parent = NULL);
    ~EditorWindow();
    virtual void append(const QString& msg, const QString& pos);
private slots:
    void on_actionClear_triggered();

    void on_action_New_triggered();

    void on_action_Run_File_triggered();
    void onEngineStarted();
    void onEngineStopped();
    void onEnginePanic(Atom file,
                       Word line,
                       const QString& error,
                       const QString& status);
    void on_action_Inspect_Selection_triggered();

    void on_action_Quit_triggered();

    void on_action_Save_triggered();

    void on_actionSave_as_triggered();

    void on_action_Open_triggered();

private:
    Ui::EditorWindow* ui;
    Highlighter* highlighter;
    QString currentFile;
    Engine* engine;
    QLabel* status;
};

#endif // EDITORWINDOW_H
