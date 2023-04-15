cmake_minimum_required(VERSION 3.23)
project(credis C)

set(CMAKE_C_STANDARD 99)

add_executable(credis main.c credis.c sc_sock.c sc_uri.c)
add_dependencies(credis credis)