cmake_minimum_required(VERSION 3.27)

project(rdft LANGUAGES C)

# OpenCL
add_compile_definitions(CL_TARGET_OPENCL_VERSION=300)
find_package(OpenCL REQUIRED)

add_executable(${PROJECT_NAME} rdft.c)
target_link_libraries(${PROJECT_NAME} OpenCL::OpenCL m)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

configure_file(rdft.cl ${CMAKE_CURRENT_BINARY_DIR}/ COPYONLY)
