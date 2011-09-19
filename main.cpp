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

MainWindow* window;

int main(int argc, char *argv[])
{    
    QApplication a(argc, argv);
    window = new MainWindow();
    window->showMaximized();


    return a.exec();
}


