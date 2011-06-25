#ifndef TOOLS_H
#define TOOLS_H

#include <iostream>
#include <QThread>

#define TRACE(what) std::wcout << "[" << QThread::currentThreadId() << "] " << what << std::endl;


#endif // TOOLS_H
