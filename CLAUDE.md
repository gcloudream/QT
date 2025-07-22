# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a Qt-based 3D point cloud processing application with floorplan generation capabilities. The application combines Qt OpenGL widgets for 3D visualization with computer vision algorithms for architectural floorplan extraction from point cloud data.

### Main Application Structure
- **Main Qt Application**: Located in root directory, built using qmake with `demoC.pro`
- **Floorplan Processing Module**: Separate C++ module in `floorplan_code_v1/` with CMake build system
- **Python Scripts**: Visualization and preprocessing scripts in `floorplan_code_v1/py_script/`

## Build System & Commands

### Qt Main Application (qmake)
```bash
# Build the main Qt application
qmake demoC.pro
make

# Note: vcpkg dependencies are configured in demoC.pro for:
# - Assimp (3D model loading)
# - FreeGLUT (OpenGL utilities) 
# - DevIL (image loading)
# - OpenGL libraries
```

### Floorplan Segmentation Module (CMake)
```bash
# Navigate to segmentation module
cd floorplan_code_v1/Segmentation

# Build with CMake
mkdir -p build && cd build
cmake ..
make

# Dependencies configured in CMakeLists.txt:
# - OpenCV (computer vision)
# - CGAL (computational geometry)
# - GMP (arbitrary precision arithmetic)
```

## Architecture & Code Structure

### 3D Visualization Components
- **MyQOpenglWidget** (`myqopenglwidget.h/cpp`): Main OpenGL rendering widget with point cloud visualization, camera controls, and shader management
- **PCDReader** (`pcdreader.h/cpp`): Comprehensive PCD file format reader supporting ASCII, Binary, and Binary_Compressed formats
- **MinBoundingBox** (`minboundingbox.h/cpp`): Geometric utilities for 3D bounding box calculations
- **ModelManager** (`modelmanager.h/cpp`): 3D model loading and management

### Main Application Window
- **MainWindow** (`mainwindow.h/cpp`): Central application controller with file system integration, multiple view modes (original 3D view and line plot view), and project management
- **LineplotWidget** (`lineplotwidget.h/cpp`): 2D plotting widget for data analysis visualization

### Floorplan Processing Pipeline
The floorplan generation involves a multi-stage computer vision pipeline:

1. **Point Cloud Segmentation** (`floorplan_code_v1/Segmentation/`):
   - RANSAC-based plane detection (`ransac_detection.cpp`)
   - Geometric primitive extraction using CGAL
   - PLY file I/O for mesh data (`ply.h/ply.c`)

2. **Python Processing Scripts** (`floorplan_code_v1/py_script/`):
   - `bash_run.py`: Pipeline orchestration
   - `main_generate_wall_vis.py`: Wall boundary extraction and visualization
   - `normal_estimation.py`: Surface normal computation for point clouds
   - `floorplan_vis.py`: Final floorplan visualization
   - `pcd2xyz.py`: Point cloud format conversion utilities

### Data Flow Architecture
1. **Input**: PCD/PLY point cloud files loaded through PCDReader
2. **3D Visualization**: Points rendered in MyQOpenglWidget with OpenGL shaders
3. **Processing**: Point cloud passed to floorplan_code_v1 segmentation module
4. **Analysis**: Python scripts process segmented data to extract architectural features
5. **Output**: Generated floorplan data and visualizations

## Key Integration Points

- **File System Integration**: MainWindow provides Qt-based file browser for managing point cloud datasets
- **Multi-View Architecture**: Application supports switching between 3D point cloud view and 2D analysis plots
- **Cross-Module Communication**: MainWindow coordinates between Qt visualization and external C++/Python processing modules
- **OpenGL Pipeline**: Custom shader programs handle point cloud rendering with configurable visualization modes

## Development Notes

- **vcpkg Dependencies**: Windows-specific paths are hardcoded in demoC.pro - may need adjustment for different environments
- **Build Dependencies**: Ensure OpenCV, CGAL, and GMP are available for the segmentation module
- **Python Environment**: Floorplan processing scripts require numpy, matplotlib, and other computer vision libraries
- **Static Linking**: Segmentation module configured for static linking (`CMAKE_EXE_LINKER_FLAGS "-static"`)