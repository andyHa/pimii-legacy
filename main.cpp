#include <QtCore/QCoreApplication>

#include <string>
#include <sstream>
#include <iostream>
#include <cassert>

#include "env.h"
#include "engine.h"
#include "compiler.h"

int main(int argc, char *argv[])
{    
    std::wcout << "pimii v1.0 [" << NUMBER_OF_BITS << "bit] (c) 2011 Andreas Haufler"  << std::endl;
    Engine e;
    std::wstringstream ss;
//    ss << "(#LDC 3 #LDC '4' " << std::endl <<" #BAP $parse #ADD #BAP $println)";
    ss << "$println((3 + 4) * 5 - 85)";
    Compiler c(ss, &e);
    try {
        Atom xx = c.compile();
        std::wcout << e.toString(xx) << std::endl;
        std::wcout << e.toString(e.exec(xx)) << std::endl;
    } catch(ParseException* e) {
        std::wcout << e->line << ":" << e->pos << ": " << e->error;
    }

    return 0;
//    QCoreApplication a(argc, argv);
//    return a.exec();
}
