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
    pcdreader.cpp \
    src/wall_extraction/wall_extraction_manager.cpp \
    src/wall_extraction/line_drawing_tool.cpp \
    src/wall_extraction/wall_fitting_algorithm.cpp \
    src/wall_extraction/wireframe_generator.cpp \
    src/wall_extraction/las_reader.cpp \
    src/wall_extraction/point_cloud_processor.cpp \
    src/wall_extraction/point_cloud_lod_manager.cpp \
    src/wall_extraction/spatial_index.cpp \
    src/wall_extraction/point_cloud_memory_manager.cpp \
    src/wall_extraction/top_down_view_renderer.cpp \
    src/wall_extraction/color_mapping_manager.cpp \
    src/wall_extraction/view_projection_manager.cpp \
    src/wall_extraction/top_down_interaction_controller.cpp \
    src/wall_extraction/stage1_demo_widget.cpp

HEADERS += \
    config.h \
    lineplotwidget.h \
    mainwindow.h \
    minboundingbox.h \
    modelmanager.h \
    myqopenglwidget.h \
    openglwindow.h \
    pcdreader.h \
    src/wall_extraction/wall_extraction_manager.h \
    src/wall_extraction/line_drawing_tool.h \
    src/wall_extraction/wall_fitting_algorithm.h \
    src/wall_extraction/wireframe_generator.h \
    src/wall_extraction/las_reader.h \
    src/wall_extraction/point_cloud_processor.h \
    src/wall_extraction/point_cloud_lod_manager.h \
    src/wall_extraction/spatial_index.h \
    src/wall_extraction/point_cloud_memory_manager.h \
    src/wall_extraction/top_down_view_renderer.h \
    src/wall_extraction/color_mapping_manager.h \
    src/wall_extraction/view_projection_manager.h \
    src/wall_extraction/top_down_interaction_controller.h \
    src/wall_extraction/stage1_demo_widget.h

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

# 暂时注释掉外部依赖以避免链接问题
# # 设置 Assimp 头文件路径
# INCLUDEPATH += C:/Users/18438/vcpkg/installed/x64-windows/include

# # 设置 Assimp 库文件路径
# LIBS += -LC:/Users/18438/vcpkg/installed/x64-windows/lib

# # 设置 include 路径，指向 FreeGLUT 的头文件所在目录
# INCLUDEPATH += C:/Users/18438/vcpkg/installed/x64-windows/include

# # 设置库文件路径，指向 FreeGLUT 的库文件所在目录
# LIBS += -LC:/Users/18438/vcpkg/installed/x64-windows/lib

# # 链接 Assimp 库
# LIBS += -lassimp-vc143-mt






# RESOURCES += \
#     resources.qrc

# 只保留基本的OpenGL库
LIBS += -lopengl32 -lglu32

# 暂时注释掉其他外部依赖
# # 链接 OpenGL 和 FreeGLUT 库
# LIBS += -lfreeglut  # FreeGLUT 库

# # 设置 DevIL 头文件路径
# INCLUDEPATH += $$VCPKG_PATH/include/IL

# # 链接 DevIL 库
# LIBS += -lDevIL
# LIBS += -lILU  # 同时链接IL和ILU
