cmake_minimum_required(VERSION 3.27)

project(lscl LANGUAGES C)

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

# json-c
find_package(json-c CONFIG)

add_executable(${PROJECT_NAME} lscl.c device.c deviceCL.c error.c deviceJson.c platformJson.c platform.c platformCL.c version.c)
target_link_libraries(${PROJECT_NAME} PRIVATE json-c::json-c)
