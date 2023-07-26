/*
 * Copyright 2023, Amritpal Singh
 * This file is part of icetea+.
 *
 * Foobar is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Foobar is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Foobar. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef SOCKET_ABSTACTION_H
#define SOCKET_ABSTACTION_H

/* The goal of this thin wrapper is to make the known berkely socket
 * api to be usable on windows and any other platform. Known different
 * types are aliased, and other values may be specified as preprocessor macros
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

/* introduce alias types on windows */
using SOCKDATA = WSADATA;
using icy_socket = SOCKET;
}

#else

#include <arpa/inet.h>
#include <sys/sockets.h>

namespace icysock {
#define BAD_SOCKET -1

/* introduce alias types on windows */
using SOCKDATA = int;
using icySocket = int;
}

#endif

namespace icysock {

/* initialise the sockets library
 * this is only needed on windows but this should be called
 * before using this abstraction on a platform as it may setup
 * additional structures and variables not part of the windows
 * API
 */
SOCKDATA
init();

/* terminate the sockets library
 * this is only needed on windows but this should be called
 * after being done with the API, because it may cleanup
 * additional structures and variables not part of the windows
 * API
 */
void
terminate();

}
#endif