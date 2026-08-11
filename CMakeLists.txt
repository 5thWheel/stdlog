cmake_minimum_required(VERSION 3.23)

project("stdlog" 
	VERSION "0.15.0"
	DESCRIPTION "Header-only C++20 Rolling File logger class based on fmtlib" 
	LANGUAGES CXX
)

set(CMAKE_CONFIGURATION_TYPES
	"Debug"
	"Release"
	CACHE STRING "" FORCE
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
	fmt
	GIT_REPOSITORY 	https://github.com/fmtlib/fmt.git
	GIT_TAG			12.2.0
	GIT_SHALLOW		TRUE
	SOURCE_DIR 		"${CMAKE_SOURCE_DIR}/ext/fmt"
)

FetchContent_MakeAvailable(fmt)

include(GNUInstallDirs)

# Export an interface (header-only) library
add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME} INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Optional
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_20)

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
option(BUILD_TESTING "Build sample executables" ON)

if (BUILD_TESTING)
	#add_executable(logger_example example_usage.cpp)
	add_executable(logger_example advanced_example.cpp)

	target_compile_definitions(logger_example PRIVATE 
		"$<$<CONFIG:Debug>:"
			"_CRT_SECURE_NO_WARNINGS"
			"_DEBUG"
		">"
		"$<$<CONFIG:Release>:NDEBUG>"
		"_CONSOLE"
		"UNICODE"
		"_UNICODE"
	)

	# For MSVC
	if(MSVC)
		target_compile_options(logger_example PRIVATE /std:c++20 /Zc:preprocessor)
	endif()

	# Link against standard library features
	target_link_libraries(logger_example PRIVATE stdlog fmt::fmt)

	# Optional: Add include directory
	target_include_directories(logger_example PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
endif()