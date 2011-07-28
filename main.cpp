#include <QtCore/QCoreApplication>
#include <QDir>

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <cassert>
#include <cstdlib>

#include "vm/env.h"
#include "vm/engine.h"
#include "compiler.h"
#include "gui/mainwindow.h"

#include <QApplication>

MainWindow* window;

int main(int argc, char *argv[])
{    
    QApplication a(argc, argv);
    window = new MainWindow();
    //e.kickstart(String(L"test.pi"));

    window->showMaximized();


    return a.exec();
}


