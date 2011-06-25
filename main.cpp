#include <QtCore/QCoreApplication>

#include <string>
#include <sstream>
#include <iostream>
#include <cassert>

#include "env.h"
#include "storage.h"
#include "bytecodeparser.h"
#include "engine.h"
int main(int argc, char *argv[])
{    
    try {
    std::wcout << "Number of bits in a word: " << NUMBER_OF_BITS << std::endl;
    std::wcout << "Max index size " << MAX_INDEX_SIZE << std::endl;
    std::wcout << "Small int [" << MIN_SMALL_INT_SIZE << ".." << MAX_SMALL_INT_SIZE << "]" << std::endl;
    Engine e;
    std::wstringstream ss;
    ss << "(#LDC 3 #LDC '4' #BAP $parse #ADD #BAP $println)";
//    ss << "(#LDC 3 #LDC 4 #ADD)";
    e.eval(ss);
    } catch(char const* c) {
        std::cout << std::string(c);
    }

    return 0;
//    QCoreApplication a(argc, argv);
//    return a.exec();
}
