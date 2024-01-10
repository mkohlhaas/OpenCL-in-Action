cmake_minimum_required(VERSION 3.27)

project(polarRect LANGUAGES C)

# OpenCL
add_compile_definitions(CL_TARGET_OPENCL_VERSION=300)
find_package(OpenCL REQUIRED)
link_libraries(${OpenCL_LIBRARIES})

add_executable(${PROJECT_NAME} polar_rect.c)
configure_file(polar_rect.cl ${CMAKE_CURRENT_BINARY_DIR}/ COPYONLY)
