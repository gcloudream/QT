/****************************************************************************
** Meta object code from reading C++ file 'line_drawing_tool.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.7.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../src/wall_extraction/line_drawing_tool.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'line_drawing_tool.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.7.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {

#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASSWallExtractionSCOPELineDrawingToolENDCLASS_t {};
constexpr auto qt_meta_stringdata_CLASSWallExtractionSCOPELineDrawingToolENDCLASS = QtMocHelpers::stringData(
    "WallExtraction::LineDrawingTool",
    "lineSegmentAdded",
    "",
    "LineSegment",
    "segment",
    "lineSegmentRemoved",
    "segmentId",
    "lineSegmentUpdated",
    "lineSegmentSelected",
    "lineSegmentDeselected",
    "polylineAdded",
    "Polyline",
    "polyline",
    "polylineRemoved",
    "polylineId",
    "polylineUpdated",
    "drawingModeChanged",
    "DrawingMode",
    "mode",
    "editModeChanged",
    "EditMode",
    "operationStarted",
    "operation",
    "operationCompleted",
    "operationCancelled",
    "errorOccurred",
    "error",
    "warningOccurred",
    "warning",
    "onInternalUpdate"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSWallExtractionSCOPELineDrawingToolENDCLASS[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
      16,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      15,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,  110,    2, 0x06,    1 /* Public */,
       5,    1,  113,    2, 0x06,    3 /* Public */,
       7,    2,  116,    2, 0x06,    5 /* Public */,
       8,    1,  121,    2, 0x06,    8 /* Public */,
       9,    1,  124,    2, 0x06,   10 /* Public */,
      10,    1,  127,    2, 0x06,   12 /* Public */,
      13,    1,  130,    2, 0x06,   14 /* Public */,
      15,    2,  133,    2, 0x06,   16 /* Public */,
      16,    1,  138,    2, 0x06,   19 /* Public */,
      19,    1,  141,    2, 0x06,   21 /* Public */,
      21,    1,  144,    2, 0x06,   23 /* Public */,
      23,    1,  147,    2, 0x06,   25 /* Public */,
      24,    1,  150,    2, 0x06,   27 /* Public */,
      25,    1,  153,    2, 0x06,   29 /* Public */,
      27,    1,  156,    2, 0x06,   31 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      29,    0,  159,    2, 0x08,   33 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, QMetaType::Int,    6,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 3,    6,    4,
    QMetaType::Void, QMetaType::Int,    6,
    QMetaType::Void, QMetaType::Int,    6,
    QMetaType::Void, 0x80000000 | 11,   12,
    QMetaType::Void, QMetaType::Int,   14,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 11,   14,   12,
    QMetaType::Void, 0x80000000 | 17,   18,
    QMetaType::Void, 0x80000000 | 20,   18,
    QMetaType::Void, QMetaType::QString,   22,
    QMetaType::Void, QMetaType::QString,   22,
    QMetaType::Void, QMetaType::QString,   22,
    QMetaType::Void, QMetaType::QString,   26,
    QMetaType::Void, QMetaType::QString,   28,

 // slots: parameters
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject WallExtraction::LineDrawingTool::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_CLASSWallExtractionSCOPELineDrawingToolENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSWallExtractionSCOPELineDrawingToolENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSWallExtractionSCOPELineDrawingToolENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<LineDrawingTool, std::true_type>,
        // method 'lineSegmentAdded'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const LineSegment &, std::false_type>,
        // method 'lineSegmentRemoved'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'lineSegmentUpdated'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<const LineSegment &, std::false_type>,
        // method 'lineSegmentSelected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'lineSegmentDeselected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'polylineAdded'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const Polyline &, std::false_type>,
        // method 'polylineRemoved'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'polylineUpdated'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<const Polyline &, std::false_type>,
        // method 'drawingModeChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<DrawingMode, std::false_type>,
        // method 'editModeChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<EditMode, std::false_type>,
        // method 'operationStarted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'operationCompleted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'operationCancelled'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'errorOccurred'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'warningOccurred'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onInternalUpdate'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void WallExtraction::LineDrawingTool::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<LineDrawingTool *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->lineSegmentAdded((*reinterpret_cast< std::add_pointer_t<LineSegment>>(_a[1]))); break;
        case 1: _t->lineSegmentRemoved((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 2: _t->lineSegmentUpdated((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<LineSegment>>(_a[2]))); break;
        case 3: _t->lineSegmentSelected((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 4: _t->lineSegmentDeselected((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 5: _t->polylineAdded((*reinterpret_cast< std::add_pointer_t<Polyline>>(_a[1]))); break;
        case 6: _t->polylineRemoved((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 7: _t->polylineUpdated((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<Polyline>>(_a[2]))); break;
        case 8: _t->drawingModeChanged((*reinterpret_cast< std::add_pointer_t<DrawingMode>>(_a[1]))); break;
        case 9: _t->editModeChanged((*reinterpret_cast< std::add_pointer_t<EditMode>>(_a[1]))); break;
        case 10: _t->operationStarted((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 11: _t->operationCompleted((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 12: _t->operationCancelled((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 13: _t->errorOccurred((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 14: _t->warningOccurred((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 15: _t->onInternalUpdate(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (LineDrawingTool::*)(const LineSegment & );
            if (_t _q_method = &LineDrawingTool::lineSegmentAdded; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (LineDrawingTool::*)(int );
            if (_t _q_method = &LineDrawingTool::lineSegmentRemoved; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (LineDrawingTool::*)(int , const LineSegment & );
            if (_t _q_method = &LineDrawingTool::lineSegmentUpdated; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (LineDrawingTool::*)(int );
            if (_t _q_method = &LineDrawingTool::lineSegmentSelected; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (LineDrawingTool::*)(int );
            if (_t _q_method = &LineDrawingTool::lineSegmentDeselected; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (LineDrawingTool::*)(const Polyline & );
            if (_t _q_method = &LineDrawingTool::polylineAdded; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (LineDrawingTool::*)(int );
            if (_t _q_method = &LineDrawingTool::polylineRemoved; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 6;
                return;
            }
        }
        {
            using _t = void (LineDrawingTool::*)(int , const Polyline & );
            if (_t _q_method = &LineDrawingTool::polylineUpdated; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 7;
                return;
            }
        }
        {
            using _t = void (LineDrawingTool::*)(DrawingMode );
            if (_t _q_method = &LineDrawingTool::drawingModeChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 8;
                return;
            }
        }
        {
            using _t = void (LineDrawingTool::*)(EditMode );
            if (_t _q_method = &LineDrawingTool::editModeChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 9;
                return;
            }
        }
        {
            using _t = void (LineDrawingTool::*)(const QString & );
            if (_t _q_method = &LineDrawingTool::operationStarted; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 10;
                return;
            }
        }
        {
            using _t = void (LineDrawingTool::*)(const QString & );
            if (_t _q_method = &LineDrawingTool::operationCompleted; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 11;
                return;
            }
        }
        {
            using _t = void (LineDrawingTool::*)(const QString & );
            if (_t _q_method = &LineDrawingTool::operationCancelled; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 12;
                return;
            }
        }
        {
            using _t = void (LineDrawingTool::*)(const QString & );
            if (_t _q_method = &LineDrawingTool::errorOccurred; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 13;
                return;
            }
        }
        {
            using _t = void (LineDrawingTool::*)(const QString & );
            if (_t _q_method = &LineDrawingTool::warningOccurred; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 14;
                return;
            }
        }
    }
}

const QMetaObject *WallExtraction::LineDrawingTool::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *WallExtraction::LineDrawingTool::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSWallExtractionSCOPELineDrawingToolENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int WallExtraction::LineDrawingTool::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 16)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 16;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 16)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 16;
    }
    return _id;
}

// SIGNAL 0
void WallExtraction::LineDrawingTool::lineSegmentAdded(const LineSegment & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void WallExtraction::LineDrawingTool::lineSegmentRemoved(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void WallExtraction::LineDrawingTool::lineSegmentUpdated(int _t1, const LineSegment & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void WallExtraction::LineDrawingTool::lineSegmentSelected(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void WallExtraction::LineDrawingTool::lineSegmentDeselected(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void WallExtraction::LineDrawingTool::polylineAdded(const Polyline & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void WallExtraction::LineDrawingTool::polylineRemoved(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void WallExtraction::LineDrawingTool::polylineUpdated(int _t1, const Polyline & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}

// SIGNAL 8
void WallExtraction::LineDrawingTool::drawingModeChanged(DrawingMode _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 8, _a);
}

// SIGNAL 9
void WallExtraction::LineDrawingTool::editModeChanged(EditMode _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 9, _a);
}

// SIGNAL 10
void WallExtraction::LineDrawingTool::operationStarted(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 10, _a);
}

// SIGNAL 11
void WallExtraction::LineDrawingTool::operationCompleted(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 11, _a);
}

// SIGNAL 12
void WallExtraction::LineDrawingTool::operationCancelled(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 12, _a);
}

// SIGNAL 13
void WallExtraction::LineDrawingTool::errorOccurred(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 13, _a);
}

// SIGNAL 14
void WallExtraction::LineDrawingTool::warningOccurred(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 14, _a);
}
QT_WARNING_POP
