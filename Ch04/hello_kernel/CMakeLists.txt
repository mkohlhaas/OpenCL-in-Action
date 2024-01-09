cmake_minimum_required(VERSION 3.27)

project(helloKernel LANGUAGES C)

# OpenCL
add_compile_definitions(CL_TARGET_OPENCL_VERSION=300)
find_package(OpenCL REQUIRED)
link_libraries(${OpenCL_LIBRARIES})

add_executable(${PROJECT_NAME} hello_kernel.c)
configure_file(hello_kernel.cl ${CMAKE_CURRENT_BINARY_DIR}/ COPYONLY)
