cmake_minimum_required(VERSION 3.27)

project(radixSort8 LANGUAGES C)

# OpenCL
add_compile_definitions(CL_TARGET_OPENCL_VERSION=300)
find_package(OpenCL REQUIRED)

add_executable(${PROJECT_NAME} radix_sort8.c)
target_link_libraries(${PROJECT_NAME} OpenCL::OpenCL)

configure_file(radix_sort8.cl ${CMAKE_CURRENT_BINARY_DIR}/ COPYONLY)
