QT += core widgets opengl openglwidgets

CONFIG += c++17

TARGET = test_stage2_ui
TEMPLATE = app

# 源文件
SOURCES += test_stage2_ui_visibility.cpp \
    src/wall_extraction/stage1_demo_widget.cpp \
    src/wall_extraction/wall_extraction_manager.cpp \
    src/wall_extraction/line_drawing_tool.cpp \
    src/wall_extraction/line_drawing_toolbar.cpp \
    src/wall_extraction/line_property_panel.cpp \
    src/wall_extraction/line_info_panel.cpp \
    src/wall_extraction/line_list_widget.cpp \
    src/wall_extraction/top_down_view_renderer.cpp \
    src/wall_extraction/color_mapping_manager.cpp \
    src/wall_extraction/point_cloud_lod_manager.cpp \
    src/wall_extraction/point_cloud_memory_manager.cpp \
    src/wall_extraction/spatial_index.cpp \
    src/wall_extraction/point_cloud_processor.cpp \
    src/wall_extraction/las_reader.cpp \
    src/wall_extraction/wall_fitting_algorithm.cpp \
    src/wall_extraction/wireframe_generator.cpp \
    src/wall_extraction/wall_fitting_progress_dialog.cpp \
    src/wall_extraction/wall_fitting_result_dialog.cpp \
    src/wall_extraction/ui_integration_helper.cpp \
    src/wall_extraction/view_projection_manager.cpp \
    src/wall_extraction/top_down_interaction_controller.cpp

# 头文件
HEADERS += src/wall_extraction/stage1_demo_widget.h \
    src/wall_extraction/wall_extraction_manager.h \
    src/wall_extraction/line_drawing_tool.h \
    src/wall_extraction/line_drawing_toolbar.h \
    src/wall_extraction/line_property_panel.h \
    src/wall_extraction/line_info_panel.h \
    src/wall_extraction/line_list_widget.h \
    src/wall_extraction/top_down_view_renderer.h \
    src/wall_extraction/color_mapping_manager.h \
    src/wall_extraction/point_cloud_lod_manager.h \
    src/wall_extraction/point_cloud_memory_manager.h \
    src/wall_extraction/spatial_index.h \
    src/wall_extraction/point_cloud_processor.h \
    src/wall_extraction/las_reader.h \
    src/wall_extraction/wall_fitting_algorithm.h \
    src/wall_extraction/wireframe_generator.h \
    src/wall_extraction/wall_fitting_progress_dialog.h \
    src/wall_extraction/wall_fitting_result_dialog.h \
    src/wall_extraction/ui_integration_helper.h \
    src/wall_extraction/view_projection_manager.h \
    src/wall_extraction/top_down_interaction_controller.h

# 包含路径
INCLUDEPATH += . src

# 编译器选项
QMAKE_CXXFLAGS += -Wno-unknown-pragmas

# Windows特定设置
win32 {
    LIBS += -lopengl32 -lglu32
}

# 输出目录
DESTDIR = test_build
OBJECTS_DIR = test_build/obj
MOC_DIR = test_build/moc
