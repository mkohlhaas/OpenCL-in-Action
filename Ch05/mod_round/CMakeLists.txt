cmake_minimum_required(VERSION 3.27)

project(modRound LANGUAGES C)

# OpenCL
add_compile_definitions(CL_TARGET_OPENCL_VERSION=300)
find_package(OpenCL REQUIRED)
link_libraries(${OpenCL_LIBRARIES})

add_executable(${PROJECT_NAME} mod_round.c)
configure_file(mod_round.cl ${CMAKE_CURRENT_BINARY_DIR}/ COPYONLY)
