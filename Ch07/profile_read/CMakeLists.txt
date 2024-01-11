cmake_minimum_required(VERSION 3.27)

project(profileRead LANGUAGES C)

# OpenCL
add_compile_definitions(CL_TARGET_OPENCL_VERSION=300)
find_package(OpenCL REQUIRED)

add_executable(${PROJECT_NAME} profile_read.c)
target_link_libraries(${PROJECT_NAME} OpenCL::OpenCL)

configure_file(profile_read.cl ${CMAKE_CURRENT_BINARY_DIR}/ COPYONLY)
