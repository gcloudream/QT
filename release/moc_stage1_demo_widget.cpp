/****************************************************************************
** Meta object code from reading C++ file 'stage1_demo_widget.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.7.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../src/wall_extraction/stage1_demo_widget.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'stage1_demo_widget.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSStage1DemoWidgetENDCLASS_t {};
constexpr auto qt_meta_stringdata_CLASSStage1DemoWidgetENDCLASS = QtMocHelpers::stringData(
    "Stage1DemoWidget",
    "loadPointCloudFile",
    "",
    "generateTestData",
    "clearPointCloud",
    "onLODLevelChanged",
    "level",
    "onLODStrategyChanged",
    "strategy",
    "generateLODLevels",
    "onColorSchemeChanged",
    "scheme",
    "onColorRangeChanged",
    "generateColorBar",
    "onRenderModeChanged",
    "mode",
    "onPointSizeChanged",
    "size",
    "renderTopDownView",
    "saveRenderResult",
    "saveCurrentImage",
    "updateRenderView"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSStage1DemoWidgetENDCLASS[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
      15,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,  104,    2, 0x08,    1 /* Private */,
       3,    0,  105,    2, 0x08,    2 /* Private */,
       4,    0,  106,    2, 0x08,    3 /* Private */,
       5,    1,  107,    2, 0x08,    4 /* Private */,
       7,    1,  110,    2, 0x08,    6 /* Private */,
       9,    0,  113,    2, 0x08,    8 /* Private */,
      10,    1,  114,    2, 0x08,    9 /* Private */,
      12,    0,  117,    2, 0x08,   11 /* Private */,
      13,    0,  118,    2, 0x08,   12 /* Private */,
      14,    1,  119,    2, 0x08,   13 /* Private */,
      16,    1,  122,    2, 0x08,   15 /* Private */,
      18,    0,  125,    2, 0x08,   17 /* Private */,
      19,    0,  126,    2, 0x08,   18 /* Private */,
      20,    0,  127,    2, 0x08,   19 /* Private */,
      21,    0,  128,    2, 0x08,   20 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    6,
    QMetaType::Void, QMetaType::Int,    8,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   11,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   15,
    QMetaType::Void, QMetaType::Double,   17,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject Stage1DemoWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_CLASSStage1DemoWidgetENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSStage1DemoWidgetENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSStage1DemoWidgetENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<Stage1DemoWidget, std::true_type>,
        // method 'loadPointCloudFile'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'generateTestData'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'clearPointCloud'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onLODLevelChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onLODStrategyChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'generateLODLevels'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onColorSchemeChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onColorRangeChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'generateColorBar'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onRenderModeChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onPointSizeChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        // method 'renderTopDownView'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'saveRenderResult'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'saveCurrentImage'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'updateRenderView'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void Stage1DemoWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<Stage1DemoWidget *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->loadPointCloudFile(); break;
        case 1: _t->generateTestData(); break;
        case 2: _t->clearPointCloud(); break;
        case 3: _t->onLODLevelChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 4: _t->onLODStrategyChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 5: _t->generateLODLevels(); break;
        case 6: _t->onColorSchemeChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 7: _t->onColorRangeChanged(); break;
        case 8: _t->generateColorBar(); break;
        case 9: _t->onRenderModeChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 10: _t->onPointSizeChanged((*reinterpret_cast< std::add_pointer_t<double>>(_a[1]))); break;
        case 11: _t->renderTopDownView(); break;
        case 12: _t->saveRenderResult(); break;
        case 13: _t->saveCurrentImage(); break;
        case 14: _t->updateRenderView(); break;
        default: ;
        }
    }
}

const QMetaObject *Stage1DemoWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Stage1DemoWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSStage1DemoWidgetENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int Stage1DemoWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 15)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 15;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 15)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 15;
    }
    return _id;
}
QT_WARNING_POP
