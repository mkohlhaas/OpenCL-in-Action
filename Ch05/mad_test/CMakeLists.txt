cmake_minimum_required(VERSION 3.27)

project(madTest LANGUAGES C)

# OpenCL
add_compile_definitions(CL_TARGET_OPENCL_VERSION=300)
find_package(OpenCL REQUIRED)
link_libraries(${OpenCL_LIBRARIES})

add_executable(${PROJECT_NAME} mad_test.c)
configure_file(mad_test.cl ${CMAKE_CURRENT_BINARY_DIR}/ COPYONLY)
