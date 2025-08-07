/****************************************************************************
** Meta object code from reading C++ file 'line_drawing_toolbar.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.7.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../src/wall_extraction/line_drawing_toolbar.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'line_drawing_toolbar.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSWallExtractionSCOPELineDrawingToolbarENDCLASS_t {};
constexpr auto qt_meta_stringdata_CLASSWallExtractionSCOPELineDrawingToolbarENDCLASS = QtMocHelpers::stringData(
    "WallExtraction::LineDrawingToolbar",
    "drawingModeChangeRequested",
    "",
    "DrawingMode",
    "mode",
    "editModeChangeRequested",
    "EditMode",
    "clearAllRequested",
    "deleteSelectedRequested",
    "saveRequested",
    "loadRequested",
    "onDrawingModeChanged",
    "onEditModeChanged",
    "onClearAllLines",
    "onDeleteSelected",
    "onSaveLines",
    "onLoadLines",
    "onLineSegmentAdded",
    "LineSegment",
    "segment",
    "onLineSegmentRemoved",
    "segmentId",
    "onLineSegmentSelected",
    "onDrawingModeButtonClicked",
    "modeId",
    "onEditModeButtonClicked"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSWallExtractionSCOPELineDrawingToolbarENDCLASS[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
      17,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,  116,    2, 0x06,    1 /* Public */,
       5,    1,  119,    2, 0x06,    3 /* Public */,
       7,    0,  122,    2, 0x06,    5 /* Public */,
       8,    0,  123,    2, 0x06,    6 /* Public */,
       9,    0,  124,    2, 0x06,    7 /* Public */,
      10,    0,  125,    2, 0x06,    8 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      11,    0,  126,    2, 0x0a,    9 /* Public */,
      12,    0,  127,    2, 0x0a,   10 /* Public */,
      13,    0,  128,    2, 0x0a,   11 /* Public */,
      14,    0,  129,    2, 0x0a,   12 /* Public */,
      15,    0,  130,    2, 0x0a,   13 /* Public */,
      16,    0,  131,    2, 0x0a,   14 /* Public */,
      17,    1,  132,    2, 0x0a,   15 /* Public */,
      20,    1,  135,    2, 0x0a,   17 /* Public */,
      22,    1,  138,    2, 0x0a,   19 /* Public */,
      23,    1,  141,    2, 0x08,   21 /* Private */,
      25,    1,  144,    2, 0x08,   23 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 6,    4,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 18,   19,
    QMetaType::Void, QMetaType::Int,   21,
    QMetaType::Void, QMetaType::Int,   21,
    QMetaType::Void, QMetaType::Int,   24,
    QMetaType::Void, QMetaType::Int,   24,

       0        // eod
};

Q_CONSTINIT const QMetaObject WallExtraction::LineDrawingToolbar::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_CLASSWallExtractionSCOPELineDrawingToolbarENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSWallExtractionSCOPELineDrawingToolbarENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSWallExtractionSCOPELineDrawingToolbarENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<LineDrawingToolbar, std::true_type>,
        // method 'drawingModeChangeRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<DrawingMode, std::false_type>,
        // method 'editModeChangeRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<EditMode, std::false_type>,
        // method 'clearAllRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'deleteSelectedRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'saveRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'loadRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onDrawingModeChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onEditModeChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onClearAllLines'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onDeleteSelected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onSaveLines'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onLoadLines'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onLineSegmentAdded'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const LineSegment &, std::false_type>,
        // method 'onLineSegmentRemoved'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onLineSegmentSelected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onDrawingModeButtonClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onEditModeButtonClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>
    >,
    nullptr
} };

void WallExtraction::LineDrawingToolbar::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<LineDrawingToolbar *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->drawingModeChangeRequested((*reinterpret_cast< std::add_pointer_t<DrawingMode>>(_a[1]))); break;
        case 1: _t->editModeChangeRequested((*reinterpret_cast< std::add_pointer_t<EditMode>>(_a[1]))); break;
        case 2: _t->clearAllRequested(); break;
        case 3: _t->deleteSelectedRequested(); break;
        case 4: _t->saveRequested(); break;
        case 5: _t->loadRequested(); break;
        case 6: _t->onDrawingModeChanged(); break;
        case 7: _t->onEditModeChanged(); break;
        case 8: _t->onClearAllLines(); break;
        case 9: _t->onDeleteSelected(); break;
        case 10: _t->onSaveLines(); break;
        case 11: _t->onLoadLines(); break;
        case 12: _t->onLineSegmentAdded((*reinterpret_cast< std::add_pointer_t<LineSegment>>(_a[1]))); break;
        case 13: _t->onLineSegmentRemoved((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 14: _t->onLineSegmentSelected((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 15: _t->onDrawingModeButtonClicked((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 16: _t->onEditModeButtonClicked((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (LineDrawingToolbar::*)(DrawingMode );
            if (_t _q_method = &LineDrawingToolbar::drawingModeChangeRequested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (LineDrawingToolbar::*)(EditMode );
            if (_t _q_method = &LineDrawingToolbar::editModeChangeRequested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (LineDrawingToolbar::*)();
            if (_t _q_method = &LineDrawingToolbar::clearAllRequested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (LineDrawingToolbar::*)();
            if (_t _q_method = &LineDrawingToolbar::deleteSelectedRequested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (LineDrawingToolbar::*)();
            if (_t _q_method = &LineDrawingToolbar::saveRequested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (LineDrawingToolbar::*)();
            if (_t _q_method = &LineDrawingToolbar::loadRequested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
    }
}

const QMetaObject *WallExtraction::LineDrawingToolbar::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *WallExtraction::LineDrawingToolbar::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSWallExtractionSCOPELineDrawingToolbarENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int WallExtraction::LineDrawingToolbar::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 17)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 17;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 17)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 17;
    }
    return _id;
}

// SIGNAL 0
void WallExtraction::LineDrawingToolbar::drawingModeChangeRequested(DrawingMode _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void WallExtraction::LineDrawingToolbar::editModeChangeRequested(EditMode _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void WallExtraction::LineDrawingToolbar::clearAllRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void WallExtraction::LineDrawingToolbar::deleteSelectedRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void WallExtraction::LineDrawingToolbar::saveRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void WallExtraction::LineDrawingToolbar::loadRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}
QT_WARNING_POP
