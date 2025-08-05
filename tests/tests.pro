QT += testlib core gui widgets openglwidgets
QT += opengl

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle
CONFIG += c++17

TEMPLATE = app

# 测试目标名称
TARGET = wall_extraction_tests

# 包含主项目的源文件路径
INCLUDEPATH += ../src/wall_extraction
INCLUDEPATH += ../

# 测试源文件
SOURCES += \
    wall_extraction_manager_test.cpp

# 包含被测试的源文件
SOURCES += \
    ../src/wall_extraction/wall_extraction_manager.cpp \
    ../src/wall_extraction/line_drawing_tool.cpp \
    ../src/wall_extraction/wall_fitting_algorithm.cpp \
    ../src/wall_extraction/wireframe_generator.cpp

# 包含被测试的头文件
HEADERS += \
    ../src/wall_extraction/wall_extraction_manager.h \
    ../src/wall_extraction/line_drawing_tool.h \
    ../src/wall_extraction/wall_fitting_algorithm.h \
    ../src/wall_extraction/wireframe_generator.h

# 链接库（与主项目保持一致）
LIBS += -lopengl32 -lglu32

# 编译器标志
QMAKE_CXXFLAGS += -Wno-unknown-pragmas
