cmake_minimum_required(VERSION 3.27)

project(glfwTst LANGUAGES C)

# OpenCL
add_compile_definitions(CL_TARGET_OPENCL_VERSION=300)
find_package(OpenCL REQUIRED)

# OpenGL
find_package(OpenGL REQUIRED)

# GLFW
find_package(glfw3 3.3 REQUIRED)

# GLAD
add_library(glad SHARED glad.c)
target_include_directories(glad PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(${PROJECT_NAME} glfwTst.c)
target_link_libraries(${PROJECT_NAME} PRIVATE OpenCL::OpenCL OpenGL::OpenGL glfw glad m)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
