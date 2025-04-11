cmake_minimum_required(VERSION 3.20)
project(lab1)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_COMPILER clang)

add_executable(server server.c)
add_executable(client client.c)