cmake_minimum_required(VERSION 3.27)

project(userEvent LANGUAGES C)

# OpenCL
add_compile_definitions(CL_TARGET_OPENCL_VERSION=300)
find_package(OpenCL REQUIRED)

add_executable(${PROJECT_NAME} user_event.c)
target_link_libraries(${PROJECT_NAME} OpenCL::OpenCL)

configure_file(user_event.cl ${CMAKE_CURRENT_BINARY_DIR}/ COPYONLY)
