cmake_minimum_required(VERSION 3.10)
project(DHCPProject CXX)

# Устанавливаем компилятор
set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Флаги компиляции
add_compile_options(-Wall -Wextra -Werror -g)

# Директория сервера
add_executable(dhcp_server
    server/server.cpp
    server/main.cpp
)
target_include_directories(dhcp_server PRIVATE server)
target_compile_features(dhcp_server PRIVATE cxx_std_17)

# Директория клиента
add_executable(dhcp_client
    client/client.cpp
    client/main.cpp
)
target_include_directories(dhcp_client PRIVATE client)
target_compile_features(dhcp_client PRIVATE cxx_std_17)