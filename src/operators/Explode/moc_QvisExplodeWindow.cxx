/****************************************************************************
** Meta object code from reading C++ file 'QvisExplodeWindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.6.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "QvisExplodeWindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'QvisExplodeWindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.6.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_QvisExplodeWindow_t {
    QByteArrayData data[18];
    char stringdata0[388];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QvisExplodeWindow_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QvisExplodeWindow_t qt_meta_stringdata_QvisExplodeWindow = {
    {
QT_MOC_LITERAL(0, 0, 17), // "QvisExplodeWindow"
QT_MOC_LITERAL(1, 18, 20), // "explosionTypeChanged"
QT_MOC_LITERAL(2, 39, 0), // ""
QT_MOC_LITERAL(3, 40, 3), // "val"
QT_MOC_LITERAL(4, 44, 25), // "explosionPointProcessText"
QT_MOC_LITERAL(5, 70, 21), // "planePointProcessText"
QT_MOC_LITERAL(6, 92, 20), // "planeNormProcessText"
QT_MOC_LITERAL(7, 113, 25), // "cylinderPoint1ProcessText"
QT_MOC_LITERAL(8, 139, 25), // "cylinderPoint2ProcessText"
QT_MOC_LITERAL(9, 165, 34), // "materialExplosionFactorProces..."
QT_MOC_LITERAL(10, 200, 30), // "cellExplosionFactorProcessText"
QT_MOC_LITERAL(11, 231, 19), // "materialProcessText"
QT_MOC_LITERAL(12, 251, 25), // "cylinderRadiusProcessText"
QT_MOC_LITERAL(13, 277, 25), // "explosionTabsChangedIndex"
QT_MOC_LITERAL(14, 303, 4), // "type"
QT_MOC_LITERAL(15, 308, 28), // "explosionPatternChangedIndex"
QT_MOC_LITERAL(16, 337, 27), // "explodeMaterialCellsToggled"
QT_MOC_LITERAL(17, 365, 22) // "explodeAllCellsToggled"

    },
    "QvisExplodeWindow\0explosionTypeChanged\0"
    "\0val\0explosionPointProcessText\0"
    "planePointProcessText\0planeNormProcessText\0"
    "cylinderPoint1ProcessText\0"
    "cylinderPoint2ProcessText\0"
    "materialExplosionFactorProcessText\0"
    "cellExplosionFactorProcessText\0"
    "materialProcessText\0cylinderRadiusProcessText\0"
    "explosionTabsChangedIndex\0type\0"
    "explosionPatternChangedIndex\0"
    "explodeMaterialCellsToggled\0"
    "explodeAllCellsToggled"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QvisExplodeWindow[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      14,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   84,    2, 0x08 /* Private */,
       4,    0,   87,    2, 0x08 /* Private */,
       5,    0,   88,    2, 0x08 /* Private */,
       6,    0,   89,    2, 0x08 /* Private */,
       7,    0,   90,    2, 0x08 /* Private */,
       8,    0,   91,    2, 0x08 /* Private */,
       9,    0,   92,    2, 0x08 /* Private */,
      10,    0,   93,    2, 0x08 /* Private */,
      11,    0,   94,    2, 0x08 /* Private */,
      12,    0,   95,    2, 0x08 /* Private */,
      13,    1,   96,    2, 0x08 /* Private */,
      15,    1,   99,    2, 0x08 /* Private */,
      16,    1,  102,    2, 0x08 /* Private */,
      17,    1,  105,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   14,
    QMetaType::Void, QMetaType::Int,   14,
    QMetaType::Void, QMetaType::Bool,    3,
    QMetaType::Void, QMetaType::Bool,    3,

       0        // eod
};

void QvisExplodeWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QvisExplodeWindow *_t = static_cast<QvisExplodeWindow *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->explosionTypeChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->explosionPointProcessText(); break;
        case 2: _t->planePointProcessText(); break;
        case 3: _t->planeNormProcessText(); break;
        case 4: _t->cylinderPoint1ProcessText(); break;
        case 5: _t->cylinderPoint2ProcessText(); break;
        case 6: _t->materialExplosionFactorProcessText(); break;
        case 7: _t->cellExplosionFactorProcessText(); break;
        case 8: _t->materialProcessText(); break;
        case 9: _t->cylinderRadiusProcessText(); break;
        case 10: _t->explosionTabsChangedIndex((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 11: _t->explosionPatternChangedIndex((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 12: _t->explodeMaterialCellsToggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 13: _t->explodeAllCellsToggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject QvisExplodeWindow::staticMetaObject = {
    { &QvisOperatorWindow::staticMetaObject, qt_meta_stringdata_QvisExplodeWindow.data,
      qt_meta_data_QvisExplodeWindow,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *QvisExplodeWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QvisExplodeWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_QvisExplodeWindow.stringdata0))
        return static_cast<void*>(const_cast< QvisExplodeWindow*>(this));
    return QvisOperatorWindow::qt_metacast(_clname);
}

int QvisExplodeWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QvisOperatorWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 14)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 14;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 14)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 14;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
