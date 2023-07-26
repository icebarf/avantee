cmake_minimum_required(VERSION 3.14)
project(icetea+ VERSION 0.0.1)

set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


add_executable(icetea+)
target_sources(icetea+ PRIVATE src/generic_sockets.cpp
              )
target_include_directories(icetea+ PRIVATE include/)
target_compile_options(icetea+ PUBLIC -Wall -Wextra -Werror -Wpedantic)