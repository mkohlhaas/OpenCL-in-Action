cmake_minimum_required(VERSION 3.27)

project(progBuild LANGUAGES C)

# OpenCL
add_compile_definitions(CL_TARGET_OPENCL_VERSION=300)
find_package(OpenCL REQUIRED)
link_libraries(${OpenCL_LIBRARIES})

add_executable(${PROJECT_NAME} program_build.c)
configure_file(good.cl ${CMAKE_CURRENT_BINARY_DIR}/ COPYONLY)
configure_file(bad.cl  ${CMAKE_CURRENT_BINARY_DIR}/ COPYONLY)
