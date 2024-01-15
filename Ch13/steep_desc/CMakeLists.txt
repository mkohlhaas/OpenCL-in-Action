cmake_minimum_required(VERSION 3.27)

project(steepDesc LANGUAGES C)

# OpenCL
add_compile_definitions(CL_TARGET_OPENCL_VERSION=300)
find_package(OpenCL REQUIRED)

add_library(mmio SHARED mmio.c)
target_include_directories(mmio PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(${PROJECT_NAME} steep_desc.c mmio.c)
target_link_libraries(${PROJECT_NAME} OpenCL::OpenCL m)
target_link_libraries(${PROJECT_NAME} mmio)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

configure_file(bcsstk05.mtx ${CMAKE_CURRENT_BINARY_DIR}/ COPYONLY)
configure_file(steep_desc.cl ${CMAKE_CURRENT_BINARY_DIR}/ COPYONLY)
