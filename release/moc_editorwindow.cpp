/****************************************************************************
** Meta object code from reading C++ file 'editorwindow.h'
**
** Created: Sun 25. Dec 16:41:29 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../gui/editorwindow.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'editorwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_EditorWindow[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      14,   13,   13,   13, 0x08,
      41,   13,   13,   13, 0x08,
      67,   13,   13,   13, 0x08,
      98,   13,   13,   13, 0x08,
     116,   13,   13,   13, 0x08,
     157,  134,   13,   13, 0x08,
     198,   13,   13,   13, 0x08,
     238,   13,   13,   13, 0x08,
     265,   13,   13,   13, 0x08,
     292,   13,   13,   13, 0x08,
     321,   13,   13,   13, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_EditorWindow[] = {
    "EditorWindow\0\0on_actionClear_triggered()\0"
    "on_action_New_triggered()\0"
    "on_action_Run_File_triggered()\0"
    "onEngineStarted()\0onEngineStopped()\0"
    "file,line,error,status\0"
    "onEnginePanic(Atom,Word,QString,QString)\0"
    "on_action_Inspect_Selection_triggered()\0"
    "on_action_Quit_triggered()\0"
    "on_action_Save_triggered()\0"
    "on_actionSave_as_triggered()\0"
    "on_action_Open_triggered()\0"
};

const QMetaObject EditorWindow::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_EditorWindow,
      qt_meta_data_EditorWindow, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &EditorWindow::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *EditorWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *EditorWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_EditorWindow))
        return static_cast<void*>(const_cast< EditorWindow*>(this));
    if (!strcmp(_clname, "Appender"))
        return static_cast< Appender*>(const_cast< EditorWindow*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int EditorWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: on_actionClear_triggered(); break;
        case 1: on_action_New_triggered(); break;
        case 2: on_action_Run_File_triggered(); break;
        case 3: onEngineStarted(); break;
        case 4: onEngineStopped(); break;
        case 5: onEnginePanic((*reinterpret_cast< Atom(*)>(_a[1])),(*reinterpret_cast< Word(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3])),(*reinterpret_cast< const QString(*)>(_a[4]))); break;
        case 6: on_action_Inspect_Selection_triggered(); break;
        case 7: on_action_Quit_triggered(); break;
        case 8: on_action_Save_triggered(); break;
        case 9: on_actionSave_as_triggered(); break;
        case 10: on_action_Open_triggered(); break;
        default: ;
        }
        _id -= 11;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
