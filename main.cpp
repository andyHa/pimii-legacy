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
        path = std::string(pimiiHome) + QDir::separator().toAscii() + "kernel.pi";
    } else {
        std::string path = (QCoreApplication::applicationDirPath() + QDir::separator() + QString("kernel.pi")).toStdString();
    }

    std::cout << "Loading Kernel: " << path  << std::endl;
    std::wifstream ss(path.c_str(), std::ios::in);
//    ss << "(#LDC 3 #LDC '4' " << std::endl <<" #BAP $parse #ADD #BAP $println)";
//    ss <<
//"if:then:else: := (cond, tb, fb) -> <<#NIL #LD (1.1) #AP #SEL (#NIL #LD (1.2) #AP) (#NIL #LD (1.3) #AP) #JOIN>>; if: [->#TRUE] then: [-> $println('Hallo') ] else: [-> $println('Welt') ]";
    Compiler c(ss, &e);
    try {
        Atom xx = c.compile();
        std::wcout << e.toString(xx) << std::endl;
        std::wcout << e.toString(e.exec(xx)) << std::endl;
    } catch(ParseException* e) {
        std::wcout << e->line << ":" << e->pos << ": " << e->error << std::endl;
    }
    ss.close();
    return 0;
//    return a.exec();
}
