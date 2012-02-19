/****************************************************************************
** Meta object code from reading C++ file 'engine.h'
**
** Created: Sun 25. Dec 16:51:31 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../vm/engine.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'engine.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Engine[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: signature, parameters, type, tag, flags
      31,    8,    7,    7, 0x05,
      82,   72,    7,    7, 0x05,
     100,    7,    7,    7, 0x05,
     122,    7,    7,    7, 0x05,
     140,    7,    7,    7, 0x05,

 // slots: signature, parameters, type, tag, flags
     188,  158,    7,    7, 0x0a,
     227,  215,    7,    7, 0x0a,
     248,    7,    7,    7, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_Engine[] = {
    "Engine\0\0file,line,error,status\0"
    "onEnginePanic(Atom,Word,QString,QString)\0"
    "file,line\0onLine(Atom,Word)\0"
    "onExecutionFinished()\0onEngineStarted()\0"
    "onEngineStopped()\0source,filename,printStackTop\0"
    "eval(QString,QString,bool)\0filename,fn\0"
    "evalFn(QString,Atom)\0fullstop()\0"
};

const QMetaObject Engine::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_Engine,
      qt_meta_data_Engine, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Engine::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Engine::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Engine::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Engine))
        return static_cast<void*>(const_cast< Engine*>(this));
    return QObject::qt_metacast(_clname);
}

int Engine::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: onEnginePanic((*reinterpret_cast< Atom(*)>(_a[1])),(*reinterpret_cast< Word(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3])),(*reinterpret_cast< const QString(*)>(_a[4]))); break;
        case 1: onLine((*reinterpret_cast< Atom(*)>(_a[1])),(*reinterpret_cast< Word(*)>(_a[2]))); break;
        case 2: onExecutionFinished(); break;
        case 3: onEngineStarted(); break;
        case 4: onEngineStopped(); break;
        case 5: eval((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3]))); break;
        case 6: evalFn((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< Atom(*)>(_a[2]))); break;
        case 7: fullstop(); break;
        default: ;
        }
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void Engine::onEnginePanic(Atom _t1, Word _t2, const QString & _t3, const QString & _t4)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)), const_cast<void*>(reinterpret_cast<const void*>(&_t4)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void Engine::onLine(Atom _t1, Word _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void Engine::onExecutionFinished()
{
    QMetaObject::activate(this, &staticMetaObject, 2, 0);
}

// SIGNAL 3
void Engine::onEngineStarted()
{
    QMetaObject::activate(this, &staticMetaObject, 3, 0);
}

// SIGNAL 4
void Engine::onEngineStopped()
{
    QMetaObject::activate(this, &staticMetaObject, 4, 0);
}
QT_END_MOC_NAMESPACE
