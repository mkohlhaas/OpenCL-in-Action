cmake_minimum_required(VERSION 3.27)

project(reductionComplete LANGUAGES C)

# OpenCL
add_compile_definitions(CL_TARGET_OPENCL_VERSION=300)
find_package(OpenCL REQUIRED)

add_executable(${PROJECT_NAME} reduction_complete.c)
target_link_libraries(${PROJECT_NAME} OpenCL::OpenCL)

configure_file(reduction_complete.cl ${CMAKE_CURRENT_BINARY_DIR}/ COPYONLY)
