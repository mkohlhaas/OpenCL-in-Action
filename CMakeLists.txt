cmake_minimum_required(VERSION 3.27)

project(opencl_in_action LANGUAGES C)

# OpenCL
add_compile_definitions(CL_TARGET_OPENCL_VERSION=300)
find_package(OpenCL REQUIRED)
link_libraries(${OpenCL_LIBRARIES})

add_subdirectory(Ch02/contextCount/)
