cmake_minimum_required(VERSION 3.27)

project(matrixMult LANGUAGES C)

# OpenCL
add_compile_definitions(CL_TARGET_OPENCL_VERSION=300)
find_package(OpenCL REQUIRED)

add_executable(${PROJECT_NAME} matrix_mult.c)
target_link_libraries(${PROJECT_NAME} OpenCL::OpenCL m)

configure_file(matrix_mult.cl ${CMAKE_CURRENT_BINARY_DIR}/ COPYONLY)
