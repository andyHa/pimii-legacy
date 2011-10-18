#include <QtCore/QCoreApplication>
#include <QDir>

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <cassert>
#include <cstdlib>

#include "gui/mainwindow.h"

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

int main(int argc, char *argv[])
{    
    QApplication a(argc, argv);
    QSettings settings("pimii","pimii");
    Engine engine(&settings);
    window = new MainWindow(&engine);
    window->showMaximized();
    engine.initialize();

    QEventLoop loop;

    while(running()) {
        a.sendPostedEvents();
        loop.processEvents(QEventLoop::WaitForMoreEvents);
        while (engine.isRunnable()) {
            engine.interpret(1000);
            a.sendPostedEvents();
            loop.processEvents(QEventLoop::AllEvents);
        }
    }
    return 0;
}


