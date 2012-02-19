/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created: Thu 10. Nov 22:41:50 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../gui/mainwindow.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_MainWindow[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
      13,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      12,   11,   11,   11, 0x0a,
      20,   11,   11,   11, 0x0a,
      35,   30,   11,   11, 0x0a,
      53,   11,   11,   11, 0x2a,
      64,   11,   11,   11, 0x0a,
      75,   30,   11,   11, 0x0a,
      95,   11,   11,   11, 0x2a,
     108,   11,   11,   11, 0x0a,
     118,   11,   11,   11, 0x0a,
     136,  128,   11,   11, 0x0a,
     160,   11,   11,   11, 0x0a,
     178,   11,   11,   11, 0x0a,
     219,  196,   11,   11, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_MainWindow[] = {
    "MainWindow\0\0about()\0newFile()\0path\0"
    "openFile(QString)\0openFile()\0saveFile()\0"
    "saveFileAs(QString)\0saveFileAs()\0"
    "runFile()\0inspect()\0msg,pos\0"
    "append(QString,QString)\0onEngineStarted()\0"
    "onEngineStopped()\0file,line,error,status\0"
    "onEnginePanic(Atom,Word,QString,QString)\0"
};

const QMetaObject MainWindow::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_MainWindow,
      qt_meta_data_MainWindow, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MainWindow::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow))
        return static_cast<void*>(const_cast< MainWindow*>(this));
    if (!strcmp(_clname, "Appender"))
        return static_cast< Appender*>(const_cast< MainWindow*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: about(); break;
        case 1: newFile(); break;
        case 2: openFile((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: openFile(); break;
        case 4: saveFile(); break;
        case 5: saveFileAs((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 6: saveFileAs(); break;
        case 7: runFile(); break;
        case 8: inspect(); break;
        case 9: append((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 10: onEngineStarted(); break;
        case 11: onEngineStopped(); break;
        case 12: onEnginePanic((*reinterpret_cast< Atom(*)>(_a[1])),(*reinterpret_cast< Word(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3])),(*reinterpret_cast< const QString(*)>(_a[4]))); break;
        default: ;
        }
        _id -= 13;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
