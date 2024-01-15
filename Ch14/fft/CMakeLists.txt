cmake_minimum_required(VERSION 3.27)

project(fft LANGUAGES C)

# OpenCL
add_compile_definitions(CL_TARGET_OPENCL_VERSION=300)
find_package(OpenCL REQUIRED)

add_executable(${PROJECT_NAME} fft.c)
target_link_libraries(${PROJECT_NAME} OpenCL::OpenCL m)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

configure_file(fft.cl ${CMAKE_CURRENT_BINARY_DIR}/ COPYONLY)
