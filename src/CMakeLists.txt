# Populate a CMake variable with the sources
set(ArenaToolBox_SRCS
        assets/FileType.cpp
        assets/Cfa.cpp
        assets/Dfa.cpp
        assets/Img.cpp
        assets/Palette.cpp
        bsa/BsaArchive.cpp
        bsa/BsaFile.cpp
        configuration/ApplicationConfiguration.cpp
        configuration/ArchiveConfiguration.cpp
        configuration/ArchiveConfigurationLoader.cpp
        configuration/FileConfiguration.cpp
        error/Status.cpp
        utils/Compression.cpp
        utils/FileUtils.cpp
        utils/HuffmanTree.cpp
        utils/BitsStreams.cpp
        utils/StreamUtils.cpp)

set(ArenaToolBox_HEADERS
        assets/FileType.h
        assets/Cfa.h
        assets/Dfa.h
        assets/Img.h
        assets/Palette.h
        bsa/BsaArchive.h
        bsa/BsaFile.h
        configuration/ApplicationConfiguration.h
        configuration/ArchiveConfiguration.h
        configuration/ArchiveConfigurationLoader.h
        configuration/FileConfiguration.h
        designpatterns/Singleton.h
        error/Status.h
        utils/Compression.h
        utils/FileUtils.h
        utils/HuffmanTree.h
        utils/BitsStreams.h
        utils/SlidingWindow.h
        utils/StreamUtils.h)

# Build library
add_library(ArenaToolBox STATIC ${ArenaToolBox_HEADERS} ${ArenaToolBox_SRCS})
target_link_libraries(ArenaToolBox Qt5::Core Qt5::Gui Qt5::Concurrent)

# Install library
# require the cmake argument: -DCMAKE_INSTALL_PREFIX=X:\\some\\path
INSTALL(TARGETS ArenaToolBox LIBRARY)
INSTALL(DIRECTORY .
        DESTINATION include
        FILES_MATCHING PATTERN "*.h")