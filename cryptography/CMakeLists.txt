cmake_minimum_required(VERSION 3.10)
project(cryptography)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SRC "cryptography_lib/*.cpp")

add_executable(cryptography
    main.cpp
    ${SRC}
)

target_include_directories(cryptography PRIVATE cryptography_lib)