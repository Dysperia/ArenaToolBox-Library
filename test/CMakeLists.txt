find_package(Qt5 COMPONENTS Test REQUIRED)

include_directories(../src)

# Populate a CMake variable with the sources
set(ArenaToolBoxTest_SRCS
        utils/CompressionTest.cpp
        utils/CompressionTest.h
        main/main.cpp)
# Tell CMake to create the executable
add_executable(ArenaToolBoxTest ${ArenaToolBoxTest_SRCS})
# Use the modules from Qt 5
target_link_libraries(ArenaToolBoxTest ArenaToolBox Qt5::Test)