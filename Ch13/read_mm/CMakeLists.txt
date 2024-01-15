cmake_minimum_required(VERSION 3.27)

project(readmm LANGUAGES C)

add_library(mmio SHARED mmio.c)
target_include_directories(mmio PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(${PROJECT_NAME} read_mm.c mmio.c)
target_link_libraries(${PROJECT_NAME} mmio)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

configure_file(bcsstk05.mtx ${CMAKE_CURRENT_BINARY_DIR}/ COPYONLY)
