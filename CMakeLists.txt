cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "iOS" OR IOS)
    set(CMAKE_OSX_ARCHITECTURES "arm64")
else()
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

project(spout4gd VERSION 1.0.0 LANGUAGES CXX)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)
add_library(${PROJECT_NAME} SHARED ${SOURCES})

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Please define GEODE_SDK environment variable to point to Geode")
else()
    message(STATUS "Found Geode: $ENV{GEODE_SDK}")
endif()

if (MSVC)
    add_compile_options(/arch:AVX) 
else()
    add_compile_options(-mssse3)
endif()

# Spout2 
set(SKIP_INSTALL_ALL ON CACHE BOOL "" FORCE)
add_subdirectory(lib/Spout2)
target_link_libraries(${PROJECT_NAME} Spout_static)
target_compile_definitions(${PROJECT_NAME} PRIVATE SPOUT_BUILD_STATIC USE_GLEW)

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)
setup_geode_mod(${PROJECT_NAME})