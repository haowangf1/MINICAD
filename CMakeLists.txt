cmake_minimum_required(VERSION 3.24)
project(minicad LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# For editor navigation (clangd / cpptools). Some generators may ignore this.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Make Qt codegen consistent for both Qt5/Qt6.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Qt (prefer Qt6, fallback to Qt5)
find_package(Qt6 COMPONENTS Widgets QUIET)
if (Qt6_FOUND)
  set(MINICAD_QT_LIB Qt6::Widgets)
  qt_standard_project_setup()
else()
  find_package(Qt5 REQUIRED COMPONENTS Widgets)
  set(MINICAD_QT_LIB Qt5::Widgets)
endif()

# OCCT
find_package(OpenCASCADE REQUIRED)

add_executable(minicad WIN32
  src/main.cpp
  src/app/MainWindow.h
  src/app/MainWindow.cpp
  src/commands/ICommand.h
  src/commands/CommandManager.h
  src/commands/CommandManager.cpp
  src/commands/CreatePrimitiveCommand.h
  src/commands/CreatePrimitiveCommand.cpp
  src/commands/ImportStepCommand.h
  src/commands/ImportStepCommand.cpp
  src/model/SceneObject.h
  src/model/Document.h
  src/model/Document.cpp
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

