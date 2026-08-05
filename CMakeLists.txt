cmake_minimum_required(VERSION 3.23)
project(RollingLogger CXX)

# Set C++ standard to C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add the example executable
add_executable(logger_example example_usage.cpp)

# Link against standard library features
target_link_libraries(logger_example PRIVATE)

# For GCC/Clang, ensure we have the right flags
if(UNIX AND NOT APPLE)
    target_compile_options(logger_example PRIVATE -std=c++23)
endif()

# For MSVC
if(MSVC)
    target_compile_options(logger_example PRIVATE /std:c++latest)
endif()

# Optional: Add include directory
target_include_directories(logger_example PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
