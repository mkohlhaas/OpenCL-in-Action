cmake_minimum_required(VERSION 3.27)

project(kernelSearch LANGUAGES C)

# OpenCL
add_compile_definitions(CL_TARGET_OPENCL_VERSION=300)
find_package(OpenCL REQUIRED)
link_libraries(${OpenCL_LIBRARIES})

# Glib2
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB2 REQUIRED glib-2.0)
include_directories(${GLIB2_INCLUDE_DIRS})
link_directories(${GLIB2_LIBRARY_DIRS})
link_libraries(${GLIB2_LIBRARIES})

add_executable(${PROJECT_NAME} kernelSearch.c aux.c)
configure_file(test.cl ${CMAKE_CURRENT_BINARY_DIR}/ COPYONLY)
