cmake_minimum_required(VERSION 2.8)
project(7zl)

set(7zl_SRCS
    7zl.cpp)

include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})
add_executable( 7zl ${7zl_SRCS} )
install(TARGETS 7zl DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)