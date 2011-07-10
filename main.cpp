#include <QtCore/QCoreApplication>
#include <QDir>

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <cassert>
#include <cstdlib>

#include "env.h"
#include "engine.h"
#include "compiler.h"

int main(int argc, char *argv[])
{    
    QCoreApplication a(argc, argv);
    std::wcout << "pimii v1.0 [" << NUMBER_OF_BITS << "bit] (c) 2011 Andreas Haufler"  << std::endl;
    Engine e;
    std::string path;
    char* pimiiHome = getenv("PIMII_HOME");
    if (pimiiHome != NULL) {
        path = std::string(pimiiHome) + QDir::separator().toAscii();
    } else {
        std::string path = (QCoreApplication::applicationDirPath() + QDir::separator()).toStdString();
    }
    e.addSourcePath(asString(path));
    e.kickstart(String(L"test.pi"));
    return 0;
}
