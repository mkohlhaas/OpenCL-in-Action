add_executable(kernelSearch kernelSearch.c aux.c)
configure_file(test.cl ${CMAKE_CURRENT_BINARY_DIR}/ COPYONLY)
