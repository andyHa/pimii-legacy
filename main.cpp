#include <QtCore/QCoreApplication>
#include <QDir>

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <cassert>
#include <cstdlib>

#include "gui/mainwindow.h"
#include "tools/logger.h"

#include <QApplication>
#include <QEventLoop>

MainWindow* window;

/**
  Determines if the program is still running - YES! Nowadays that's not so
  easy to tell: If the MainWindow is still open - we're fine, don't check any
  further. If not, check if there are visible top level widgets left.
  */
bool running() {
    if (window->isVisible()) {
        return true;
    }

    QWidgetList list = QApplication::topLevelWidgets();
    for (int i = 0; i < list.size(); ++i) {
        QWidget *w = list.at(i);
        if ((w->isVisible() &&
             !w->testAttribute(Qt::WA_DontShowOnScreen))
             && !w->parentWidget() &&
             w->testAttribute(Qt::WA_QuitOnClose)) {
                   return true;
        }
    }
    return false;
}

void loadStartupScript(Engine& engine) {
    QFileInfo startScript = QFileInfo(engine.home().
                                      absoluteFilePath("start.pi"));
    if (!startScript.exists()) {
        startScript =  QFileInfo("start.pi");
    }
    if (startScript.exists()) {
        QFile file(startScript.absoluteFilePath());
        if (file.open(QFile::ReadOnly | QFile::Text)) {
            engine.eval(file.readAll(), startScript.fileName());
        }
    }
}

void eventLoop(QApplication& a, Engine& engine) {
    QEventLoop loop;

    while(running()) {
        a.sendPostedEvents();
        loop.processEvents(QEventLoop::WaitForMoreEvents);
        while (engine.isRunnable()) {
            engine.interpret();
            a.sendPostedEvents();
            loop.processEvents(QEventLoop::AllEvents);
        }
    }
}

int main(int argc, char *argv[])
{    
    QApplication a(argc, argv);
    QSettings settings("pimii","pimii");
    Engine engine(&settings);
    window = new MainWindow(&engine);
    window->showMaximized();
    engine.initialize();

    Logger::setLevel("STORE", INFO);

    // Tries to find and load the "start.pi" file.
    loadStartupScript(engine);

    // Runs the QT eventloop interleaved with the execution of the pimii
    // engine.
    eventLoop(a, engine);

    return 0;
}


