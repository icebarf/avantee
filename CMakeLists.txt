cmake_minimum_required(VERSION 3.25)
project(icetea+ VERSION 0.0.1)

set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


add_executable(server)
target_sources(server PRIVATE src/generic_sockets.cpp
                          PRIVATE testing/test-server.cpp
              )
target_include_directories(server PRIVATE include/)
if(${CMAKE_HOST_WIN32})
  target_link_libraries(server ws2_32 )
endif()
target_compile_options(server PUBLIC -Wall -Wextra -Werror -Wpedantic
                                      -Wshadow -Wconversion)

add_executable(client)
target_sources(client PRIVATE src/generic_sockets.cpp
                          PRIVATE testing/test-client.cpp
              )
if(${CMAKE_HOST_WIN32})
  target_link_libraries(client ws2_32 mswsock advapi32)
endif()
target_include_directories(client PRIVATE include/)
target_compile_options(client PUBLIC -Wall -Wextra -Werror -Wpedantic
                                      -Wshadow -Wconversion)