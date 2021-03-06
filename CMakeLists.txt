cmake_minimum_required(VERSION 3.20.0)

project(ArenaToolBox_Library VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x060000)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)

# Find the Qt library
find_package(Qt5 COMPONENTS Core REQUIRED)
find_package(Qt5 COMPONENTS Gui REQUIRED)
find_package(Qt5 COMPONENTS Concurrent REQUIRED)

# BSATool
add_subdirectory(src)

# Tests
add_subdirectory(test)
enable_testing()
add_test(NAME ArenaToolBoxTest COMMAND BSAToolTest)