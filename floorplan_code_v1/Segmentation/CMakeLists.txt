cmake_minimum_required(VERSION 3.1)

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})

# Configuration of graph cut optimization source
# include(UseGCOptimization3)
# target_compile_definitions(libGCO PUBLIC GCO_ENERGYTYPE=double)
set(CMAKE_EXE_LINKER_FLAGS "-static")  # 完全静态链接

set(${PROJECT_NAME}_HEADERS
    base.h
    # arr.h
    ply.h
    # method_common.h
    # kdtree/kdTree.h
    # kdtree/PriorityQueue.h
    # kdtree/QueryGrid.h
    # kdtree/vector2D.h
    # kdtree/vector3D.h
    )

set(${PROJECT_NAME}_SOURCES
    main.cpp
    FileIO.cpp
    # hypothesis_generator.cpp
    # kdtree_search.cpp
    ransac_detection.cpp
    # segment_selection.cpp  
    # kdtree/kdTree.cpp
    ply.c 
    # generate_fc_map.cpp
    # arr.cpp
    )

include_directories(${OpenCV_INCLUDE_DIRS})

set(CMAKE_CXX_FLAGS, "-fopenmp")

add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_HEADERS} ${${PROJECT_NAME}_SOURCES})  

# Find OpenCV
set(OpenCV_DIR  "C:/SLAM/opencv/build/x64/vc16/lib")

find_package(OpenCV REQUIRED)

# Find CGAL
set(CGAL_DIR "C:/SLAM/CGAL-5.1.4/CGAL-5.1.4")
find_package(CGAL REQUIRED)


set(GMP_ROOT "C:/SLAM/gmp-6.3.0")  # 或 "C:/vcpkg/installed/x64-mingw-dynamic"
find_package(GMP REQUIRED)

include_directories(${GMP_INCLUDE_DIRS})

# set (libraries
#      gmp
#      mpfr
#      glog)
      
set_target_properties(${PROJECT_NAME} PROPERTIES
                      FOLDER "Facade")

target_compile_definitions(${PROJECT_NAME} PRIVATE METHOD_EXPORTS)

target_link_libraries( ${PROJECT_NAME} ${OpenCV_LIBS} CGAL::CGAL ${GMP_LIBRARIES})
