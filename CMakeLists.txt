cmake_minimum_required(VERSION 3.24)
project(minicad LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt (prefer Qt6, fallback to Qt5)
find_package(Qt6 COMPONENTS Widgets QUIET)
if (Qt6_FOUND)
  set(MINICAD_QT_LIB Qt6::Widgets)
  qt_standard_project_setup()
else()
  find_package(Qt5 REQUIRED COMPONENTS Widgets)
  set(MINICAD_QT_LIB Qt5::Widgets)
  set(CMAKE_AUTOMOC ON)
  set(CMAKE_AUTORCC ON)
  set(CMAKE_AUTOUIC ON)
endif()

# OCCT
find_package(OpenCASCADE REQUIRED)

add_executable(minicad WIN32
  src/main.cpp
  src/app/MainWindow.h
  src/app/MainWindow.cpp
  src/viewport/OccViewportWidget.h
  src/viewport/OccViewportWidget.cpp
)

target_include_directories(minicad PRIVATE
  ${OpenCASCADE_INCLUDE_DIR}
  src
)

target_link_libraries(minicad PRIVATE
  ${MINICAD_QT_LIB}
  ${OpenCASCADE_LIBRARIES}
)

