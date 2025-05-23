# -*- mode: cmake-mode; coding: utf-8-unix; *-

cmake_minimum_required(VERSION 3.25)
project(icetea+ VERSION 0.0.1)

set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(server)
target_sources(server PUBLIC ../lib/socket/error_utils.cpp
                      PUBLIC ../lib/socket/generic_sockets.cpp
                      PUBLIC ../lib/socket/socket.cpp
                      PUBLIC test-server.cpp
              )
target_include_directories(server PRIVATE ../include/)
if(${CMAKE_HOST_WIN32})
  target_link_libraries(server ws2_32 )
endif()
target_compile_options(server PUBLIC -Wall -Wextra -Wpedantic
                                      -Wshadow -g -Og)

add_executable(client)
target_sources(client PUBLIC ../lib/socket/error_utils.cpp
                              PUBLIC ../lib/socket/generic_sockets.cpp
                              PUBLIC ../lib/socket/socket.cpp
                              PUBLIC test-client.cpp
              )
target_include_directories(client PRIVATE ../include/)
if(${CMAKE_HOST_WIN32})
  target_link_libraries(client ws2_32 )
endif()
target_compile_options(client PUBLIC -Wall -Wextra -Wpedantic
                                      -Wshadow -Wconversion -g -Og)


add_executable(simple-server)
target_sources(simple-server PUBLIC ../lib/socket/error_utils.cpp
                      PUBLIC ../lib/socket/generic_sockets.cpp
                      PUBLIC ../lib/socket/socket.cpp
                      PUBLIC simple-server.cpp
              )
target_include_directories(simple-server PRIVATE ../include/)
if(${CMAKE_HOST_WIN32})
  target_link_libraries(simple-server ws2_32 )
endif()
target_compile_options(simple-server PUBLIC -Wall -Wextra -Wpedantic
                                      -Wshadow -g -Og)

add_executable(simple-client)
target_sources(simple-client PUBLIC ../lib/socket/error_utils.cpp
                      PUBLIC ../lib/socket/generic_sockets.cpp
                      PUBLIC ../lib/socket/socket.cpp
                      PUBLIC simple-client.cpp
              )
target_include_directories(simple-client PRIVATE ../include/)
if(${CMAKE_HOST_WIN32})
  target_link_libraries(simple-client ws2_32 )
endif()
target_compile_options(simple-client PUBLIC -Wall -Wextra -Wpedantic
                                      -Wshadow -g -Og)
