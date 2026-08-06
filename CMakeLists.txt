cmake_minimum_required(VERSION 3.23)

project("stdlog" 
    VERSION "0.10.0" 
    DESCRIPTION "Header-only C++23 logger class with zero external dependencies" 
    LANGUAGES CXX
)

# Set C++ standard to C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(GNUInstallDirs)

# Export an interface (header-only) library
add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME} INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Optional
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_23)

install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

#install(
#    TARGETS ${PROJECT_NAME} 
#    EXPORT "${PROJECT_NAME}Targets"
#    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
#)

#install(
#    EXPORT "${PROJECT_NAME}Targets"
#    FILE "${PROJECT_NAME}Targets.cmake"
#    NAMESPACE stdlog::
#    DESTINATION lib/cmake/stdlog
#)

# Add the example executable
option(BUILD_TESTING "Build sample executables" OFF)

if (BUILD_TESTING)
    #add_executable(logger_example example_usage.cpp)
    add_executable(logger_example advanced_example.cpp)

    # Link against standard library features
    target_link_libraries(logger_example PRIVATE)

    # For GCC/Clang, ensure we have the right flags
    if(UNIX AND NOT APPLE)
        target_compile_options(logger_example PRIVATE -std=c++23)
    endif()

    # For MSVC
    if(MSVC)
        target_compile_options(logger_example PRIVATE /std:c++latest /Zc:preprocessor)
    endif()

    # Optional: Add include directory
    target_include_directories(logger_example PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
endif()