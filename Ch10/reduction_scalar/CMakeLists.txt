cmake_minimum_required(VERSION 3.27)

project(reductionScalar LANGUAGES C)

# OpenCL
add_compile_definitions(CL_TARGET_OPENCL_VERSION=300)
find_package(OpenCL REQUIRED)

add_executable(${PROJECT_NAME} reduction.c)
target_link_libraries(${PROJECT_NAME} OpenCL::OpenCL)

configure_file(reduction.cl ${CMAKE_CURRENT_BINARY_DIR}/ COPYONLY)
