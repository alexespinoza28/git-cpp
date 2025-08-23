cmake_minimum_required(VERSION 3.10)
project(git_cpp)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find all source files. We assume a Utils.cpp exists.
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Add executable
add_executable(gitlet ${SOURCES})

# Add src to include path for headers
target_include_directories(gitlet PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")

# On Apple, CommonCrypto is part of the system libraries and should be found.
# No special linking is usually required.
