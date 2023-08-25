cmake_minimum_required(VERSION 3.10)
project (DbuProject)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# set(EXECUTABLE_OUTPUT_PATH  ${PROJECT_SOURCE_DIR}/bin/)
set(CMAKE_BUILD_TYPE Debug)

ADD_LIBRARY(Dbu STATIC
    src/downloader.cpp
    src/ftp_operation.cpp
    src/ftp_util.cpp
    src/sniffer.cpp
    src/installer.cpp
    src/option.cpp
    src/updater.cpp
)

target_include_directories(Dbu
    PRIVATE
        src/include
)

TARGET_LINK_LIBRARIES( Dbu
    ACE
    pthread
)

ADD_EXECUTABLE(DbuApp
    src/main.cpp
)

target_include_directories(DbuApp
    PRIVATE
        src/include
)

TARGET_LINK_LIBRARIES( DbuApp
    Dbu
)

SET(GCC_COVERAGE_COMPILE_FLAGS "-lACE")
SET(GCC_COVERAGE_COMPILE_FLAGS "-I/usr/include/x86_64-linux-gnu/c++/9 -stdlib=libstdc++ -stdlib=libc++ -I/usr/include/c++/9 -std=c++17")


# Locate GTest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# Link runTests with what we want to test and the GTest and pthread library
add_executable(runTests test/test_installer.cpp src/installer.cpp)
target_link_libraries(runTests Dbu gtest pthread)

target_include_directories(runTests
    PRIVATE
        src/include
)

add_executable(runSnifferTests test/test_sniffer.cpp src/downloader.cpp
    src/ftp_operation.cpp
    src/ftp_util.cpp
    src/sniffer.cpp
    src/installer.cpp
    src/option.cpp
    src/updater.cpp)
target_link_libraries(runSnifferTests Dbu gtest pthread)

target_include_directories(runSnifferTests
    PRIVATE
        src/include
)
