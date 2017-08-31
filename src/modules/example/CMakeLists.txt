cmake_minimum_required(VERSION 3.8)
project(SnakeSocksCli)

set(CMAKE_CXX_STANDARD 17)

add_definitions(-DCOMPILE_SS_MODULE)

add_library(ssmod SHARED test.c)