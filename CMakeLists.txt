cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH "${ColladaTools_SOURCE_DIR}/cmake")

project(ColladaTools CXX)

set(CMAKE_BUILD_TYPE Debug)
set(EXECUTABLE_OUTPUT_PATH ${ColladaTools_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${ColladaTools_BINARY_DIR}/lib)

add_subdirectory(src)

# Packaging options

# TODO: set the following options:
#CPACK_RESSOURCE_FILE_WELCOME
#CPACK_RESSOURCE_FILE_README
set(CPACK_RESSOURCE_FILE_LICENSE "${ColladaTools_SOURCE_DIR}/LICENSE.txt"
include(CPack)
