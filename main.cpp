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
    ss << "x := 2; Add:With:And: := [a,y,z -> var phi := a+y; phi - z]; $println(Add: 1 With: (2*2) And: 34+3))";
    Compiler c(ss, &e);
    try {
        Atom xx = c.compile();
        std::wcout << e.toString(xx) << std::endl;
        std::wcout << e.toString(e.exec(xx)) << std::endl;
    } catch(ParseException* e) {
        std::wcout << e->line << ":" << e->pos << ": " << e->error << std::endl;
    }

    return 0;
//    QCoreApplication a(argc, argv);
//    return a.exec();
}
