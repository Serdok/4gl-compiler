cmake_minimum_required(VERSION 3.26)
project(4gl_parser)

set(CMAKE_CXX_STANDARD 23)

add_executable(4gl_parser main.cpp token.cpp lexer.cpp)
target_compile_options(4gl_parser PRIVATE -fconcepts-diagnostics-depth=3)