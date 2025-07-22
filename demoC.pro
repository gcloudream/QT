QT       += core gui
QT       += openglwidgets
QT += widgets
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# 禁用未知pragma警告
QMAKE_CXXFLAGS += -Wno-unknown-pragmas

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    config.cpp \
    lineplotwidget.cpp \
    main.cpp \
    mainwindow.cpp \
    minboundingbox.cpp \
    modelmanager.cpp \
    myqopenglwidget.cpp \
    openglwindow.cpp \
    pcdreader.cpp

HEADERS += \
    config.h \
    lineplotwidget.h \
    mainwindow.h \
    minboundingbox.h \
    modelmanager.h \
    myqopenglwidget.h \
    openglwindow.h \
    pcdreader.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


# # 设置 Assimp 头文件路径
# INCLUDEPATH += D:/vcpkg/vcpkg/installed/x64-windows/include

# # 设置 Assimp 库文件路径
# LIBS += -LD:/vcpkg/vcpkg/installed/x64-windows/lib

# # 设置 include 路径，指向 FreeGLUT 的头文件所在目录
# INCLUDEPATH += D:/vcpkg/vcpkg/installed/x64-windows/include

# # 设置库文件路径，指向 FreeGLUT 的库文件所在目录
# LIBS += -LD:/vcpkg/vcpkg/installed/x64-windows/lib

# 设置 Assimp 头文件路径
INCLUDEPATH += C:/Users/18438/vcpkg/installed/x64-windows/include

# 设置 Assimp 库文件路径
LIBS += -LC:/Users/18438/vcpkg/installed/x64-windows/lib

# 设置 include 路径，指向 FreeGLUT 的头文件所在目录
INCLUDEPATH += C:/Users/18438/vcpkg/installed/x64-windows/include

# 设置库文件路径，指向 FreeGLUT 的库文件所在目录
LIBS += -LC:/Users/18438/vcpkg/installed/x64-windows/lib

# 链接 Assimp 库
LIBS += -lassimp-vc143-mt






# RESOURCES += \
#     resources.qrc

# 链接 OpenGL 和 FreeGLUT 库
LIBS += -lopengl32  # Windows 上的 OpenGL 库
LIBS += -lfreeglut  # FreeGLUT 库

# 设置 DevIL 头文件路径
INCLUDEPATH += $$VCPKG_PATH/include/IL

# # 设置 DevIL 库文件路径
# LIBS += -L$$VCPKG_PATH/lib

# 链接 DevIL 库
LIBS += -lDevIL

LIBS += -lILU  # 同时链接IL和ILU

LIBS += -lopengl32 -lglu32
