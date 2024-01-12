cmake_minimum_required(VERSION 3.27)

project(stringSearch LANGUAGES C)

# OpenCL
add_compile_definitions(CL_TARGET_OPENCL_VERSION=300)
find_package(OpenCL REQUIRED)

add_executable(${PROJECT_NAME} string_search.c)
target_link_libraries(${PROJECT_NAME} OpenCL::OpenCL)

configure_file(string_search.cl ${CMAKE_CURRENT_BINARY_DIR}/ COPYONLY)
