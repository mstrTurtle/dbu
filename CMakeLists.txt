# CMakeLists files in this project can
# refer to the root source directory of the project as ${HELLO_SOURCE_DIR} and
# to the root binary directory of the project as ${HELLO_BINARY_DIR}.
cmake_minimum_required (VERSION 2.8.11)
project (HELLO)

# Recurse into the "Hello" and "Demo" subdirectories. This does not actually
# cause another cmake executable to run. The same process will walk through
# the project's entire directory structure.
SET(TARGET_SRC
    src/d.cpp
    )


ADD_EXECUTABLE(dbu
        src/d.cpp
)

TARGET_LINK_LIBRARIES( dbu
    ACE pthread
)

SET(GCC_COVERAGE_COMPILE_FLAGS "-lACE")
SET(GCC_COVERAGE_COMPILE_FLAGS "-I/usr/include/x86_64-linux-gnu/c++/9 -stdlib=libstdc++ -stdlib=libc++ -I/usr/include/c++/9 -std=c++17")