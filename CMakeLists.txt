cmake_minimum_required(VERSION 3.25)
project(icetea+ VERSION 0.0.1)

set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


add_executable(server)
target_sources(server PUBLIC lib/socket/error_utils.cpp
                      PUBLIC lib/socket/generic_sockets.cpp
                      PUBLIC lib/socket/socket.cpp
                      PUBLIC testing/test-server.cpp
              )
target_include_directories(server PRIVATE include/)
if(${CMAKE_HOST_WIN32})
  target_link_libraries(server ws2_32 )
endif()
target_compile_options(server PUBLIC -Wall -Wextra -Werror -Wpedantic
                                      -Wshadow -Wconversion -g -Og)

add_executable(client)
target_sources(client PRIVATE lib/socket/generic_sockets.cpp
                      PRIVATE lib/socket/error_utils.cpp
                      PUBLIC lib/socket/socket.cpp
                      PRIVATE testing/test-client.cpp
              )
if(${CMAKE_HOST_WIN32})
  target_link_libraries(client ws2_32 mswsock advapi32)
endif()
target_include_directories(client PRIVATE include/)
target_compile_options(client PUBLIC -Wall -Wextra -Werror -Wpedantic
                                      -Wshadow -Wconversion -g -Og)
add_executable(avantee-server)
target_sources(avantee-server PUBLIC lib/socket/error_utils.cpp
                      PUBLIC lib/socket/generic_sockets.cpp
                      PUBLIC lib/socket/socket.cpp
		      PUBLIC src/multiplexer.cpp
#                      PUBLIC src/avantee-server.cpp
              )
target_include_directories(avantee-server PRIVATE include/)
if(${CMAKE_HOST_WIN32})
  target_link_libraries(avantee-server ws2_32 )
endif()
target_compile_options(avantee-server PUBLIC -Wall -Wextra -Werror -Wpedantic
                                      -Wshadow -Wconversion -g -Og)

