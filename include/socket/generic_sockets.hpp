/*
 * Copyright 2023, Amritpal Singh
 * This file is part of icetea+.
 *
 * icetea+ is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * icetea+ is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * icetea+. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef SOCKET_ABSTACTION_H
#define SOCKET_ABSTACTION_H

/* The goal of this thin wrapper is to not bother with lower level C
 * api. Things know to be different on platform such as windows -
 * types, functions are aliased or wrapped upon, and other values may be
 * specified as preprocessor macros
 */

/* neatly avoid checking this everywhere */
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) ||                   \
  defined(__WIN32__) || defined(WIN64) || defined(_WIN64) ||                   \
  defined(__WIN64) || defined(__WIN64__)
#define ICY_ON_WINDOWS
#endif

#ifdef ICY_ON_WINDOWS

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

namespace icysock {

#define BAD_SOCKET INVALID_SOCKET
#define SOCK_ERR SOCKET_ERROR
#define SHUT_RD SD_RECEIVE
#define SHUT_WR SD_SEND
#define SHUT_RDWR SD_BOTH

/* introduce alias types on windows */
using sockdata = WSADATA;
using gsocket = SOCKET;
}

#else

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace icysock {

#define BAD_SOCKET -1
#define SOCK_ERR -1

/* introduce alias types on other platform */
using sockdata = int;
using gsocket = int;

}

#endif

#include <cstdint>
#include <errno.h>

namespace icysock {

/* Signed return type for size */
using ssize = std::intmax_t;
using size = std::uintmax_t;

/* initialise the sockets library
 * this is only needed on windows but this should be called
 * before using this abstraction on a platform as it may setup
 * additional structures and variables not part of the windows
 * API
 */
[[maybe_unused]] sockdata
init();

/* terminate the sockets library
 * this is only needed on windows but this should be called
 * after being done with the API, because it may cleanup
 * additional structures and variables not part of the windows
 * API
 */
void
terminate();

/* wrap around the windows closesocket() and berkely close()
 * API calls
 */
int
close_socket(gsocket s);

}
#endif