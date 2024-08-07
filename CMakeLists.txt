# -*- mode: cmake-mode; coding: utf-8-unix; *-

cmake_minimum_required(VERSION 3.25)
project(icetea+ VERSION 0.0.1)

set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(avantee-server)
target_sources(avantee-server PUBLIC lib/socket/error_utils.cpp
                      PUBLIC lib/socket/generic_sockets.cpp
                      PUBLIC lib/socket/socket.cpp
		      PUBLIC src/multiplexer.cpp
		      PUBLIC src/tftp.cpp
                      PUBLIC src/server.cpp
              )
target_include_directories(avantee-server PRIVATE include/)
if(${CMAKE_HOST_WIN32})
  target_link_libraries(avantee-server ws2_32 )
endif()
target_compile_options(avantee-server PUBLIC -Wall -Wextra -Wpedantic
                                      -Wshadow -g -Og)

add_executable(avantee-client)
target_sources(avantee-client PUBLIC lib/socket/error_utils.cpp
                      PUBLIC lib/socket/generic_sockets.cpp
                      PUBLIC lib/socket/socket.cpp
		      PUBLIC src/multiplexer.cpp
		      PUBLIC src/tftp.cpp
                      PUBLIC src/client.cpp
              )
target_include_directories(avantee-client PRIVATE include/)
if(${CMAKE_HOST_WIN32})
  target_link_libraries(avantee-client ws2_32 )
endif()
target_compile_options(avantee-client PUBLIC -Wall -Wextra -Wpedantic
                                      -Wshadow -Wconversion -g -Og)

