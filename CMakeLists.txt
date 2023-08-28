cmake_minimum_required(VERSION 3.10)
project(DbuProject)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_BUILD_TYPE Debug)

add_library(
  Dbu STATIC
  src/downloader.cpp
  src/ftp_operation.cpp
  src/ftp_util.cpp
  src/sniffer.cpp
  src/installer.cpp
  src/option.cpp
  src/updater.cpp)

target_include_directories(Dbu PRIVATE src/include)

target_link_libraries(Dbu ACE pthread)

add_executable(DbuApp src/main.cpp)

target_include_directories(DbuApp PRIVATE src/include)

target_link_libraries(DbuApp Dbu)

set(GCC_COVERAGE_COMPILE_FLAGS "-lACE")
set(GCC_COVERAGE_COMPILE_FLAGS
    "-I/usr/include/x86_64-linux-gnu/c++/9 -stdlib=libstdc++ -stdlib=libc++ -I/usr/include/c++/9 -std=c++17"
)

# Locate GTest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(runInstallerTests test/test_installer.cpp)
target_link_libraries(runInstallerTests Dbu gtest pthread)
target_include_directories(runInstallerTests PRIVATE src/include)

add_executable(runSnifferTests test/test_sniffer.cpp)
target_link_libraries(runSnifferTests Dbu gtest pthread)
target_include_directories(runSnifferTests PRIVATE src/include)

add_executable(runDownloaderTests test/test_downloader.cpp)
target_link_libraries(runDownloaderTests Dbu gtest pthread)
target_include_directories(runDownloaderTests PRIVATE src/include)

add_executable(runOptionTests test/test_option.cpp)
target_link_libraries(runOptionTests Dbu gtest pthread)
target_include_directories(runOptionTests PRIVATE src/include)
