cmake_minimum_required(VERSION 3.15)
project(tnclib CXX)

# Use C++ 23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Output directory
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/lib)   # Static/Dynamic library (.a, .lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/lib)   # Shared libs (.so, .dll)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)       # executeable

# Create output directory
file(MAKE_DIRECTORY
    ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
    ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

# Collect all source files
file(GLOB_RECURSE SOURCES
    ${PROJECT_SOURCE_DIR}/src/*.cpp
)

# Collect private headers
file(GLOB_RECURSE PRIVATE_HEADERS
    ${PROJECT_SOURCE_DIR}/src/*.hpp
    ${PROJECT_SOURCE_DIR}/src/*.h
)

# Create static library
add_library(tnclib STATIC ${SOURCES} ${PRIVATE_HEADERS})

# Include path
target_include_directories(tnclib
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src
)

# Install rules
install(TARGETS tnclib
    ARCHIVE DESTINATION lib
)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/ DESTINATION include)

