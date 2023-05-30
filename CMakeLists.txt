cmake_minimum_required(VERSION 3.18)

project(Calibrator LANGUAGES C CXX)

set (CMAKE_CXX_STANDARD 17)

option(GLIBCXX_USE_CXX11_ABI   "Set -D_GLIBCXX_USE_CXX11_ABI=1"       OFF)
option(STATIC_WINDOWS_RUNTIME  "Use static (MT/MTd) Windows runtime"  ON )

if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "No CMAKE_BUILD_TYPE specified, default to Release.")
    set(CMAKE_BUILD_TYPE "Release")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(OPEN3D_FOLDER "Debug")
else()
    set(OPEN3D_FOLDER "Release")
endif()

if(WIN32)
    find_package(Open3D HINTS open3d-windows-${OPEN3D_FOLDER}/CMake)
else()
    find_package(Open3D HINTS open3d-linux/lib/cmake)
endif()

add_executable(calibrator)
target_include_directories(calibrator PRIVATE include)
target_link_libraries(calibrator PRIVATE Open3D::Open3D)
target_sources(calibrator PRIVATE
    src/main.cpp
    src/main_window.cpp
    src/data.cpp
    src/gui_state.cpp
)

if(WIN32)
    get_target_property(open3d_type Open3D::Open3D TYPE)
    if(open3d_type STREQUAL "SHARED_LIBRARY")
        message(STATUS "Copying Open3D.dll from ${PROJECT_SOURCE_DIR}/open3d-windows-${OPEN3D_FOLDER}/bin/ to ${CMAKE_CURRENT_BINARY_DIR}")
        add_custom_command(TARGET calibrator POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/open3d-windows-${OPEN3D_FOLDER}/bin/Open3D.dll ${CMAKE_CURRENT_BINARY_DIR})
    endif()
endif()

message(STATUS "Copying Open3D resources folder from ${PROJECT_SOURCE_DIR}/open3d-windows-${OPEN3D_FOLDER}/bin/ to ${CMAKE_CURRENT_BINARY_DIR}")
file(COPY ${PROJECT_SOURCE_DIR}/open3d-windows-${OPEN3D_FOLDER}/bin/resources DESTINATION ${CMAKE_CURRENT_BINARY_DIR})