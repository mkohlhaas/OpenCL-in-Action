cmake_minimum_required(VERSION 3.27)

project(simpleImage LANGUAGES C)

# OpenCL
add_compile_definitions(CL_TARGET_OPENCL_VERSION=300)
find_package(OpenCL REQUIRED)

# libpng
find_package(PNG REQUIRED)

add_executable(${PROJECT_NAME} simple_image.c)
target_link_libraries(${PROJECT_NAME} OpenCL::OpenCL)
target_link_libraries(${PROJECT_NAME} PNG::PNG)

configure_file(simple_image.cl ${CMAKE_CURRENT_BINARY_DIR}/ COPYONLY)
