cmake_minimum_required(VERSION 3.3)
project(server)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp servers/uptime.c servers/uptime.h common/server.c common/server.h common/apue.c common/apue.h)
add_executable(server ${SOURCE_FILES})