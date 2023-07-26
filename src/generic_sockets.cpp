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

#include <cstdio>
#include <exception>

#include "generic_sockets.hpp"

namespace icysock {

SOCKDATA
init()
{
  int result = 0;
  SOCKDATA wsadata;
#if defined(ICY_ON_WINDOWS)
  result = WSAStartup(MAKEWORD(2, 2), &wsadata);
  if (result != 0) {
    fprintf(stderr, "icysock: WSAStartup failed with code: %d\n", result);
    perror("icysock: ");
    WSACleanup();
    return wsadata;
  }
  return wsadata;
#else
  return result;
#endif
}

void
terminate()
{
#if defined(ICY_ON_WINDOWS)
  if (int ret = WSACleanup()) {
    switch (WSAGetLastError()) {
      case WSANOTINITIALISED:
        fprintf(stderr,
                "icysock: A successful WSAStartup call did not occur!\n");
        break;

      case WSAENETDOWN:
        fprintf(stderr, "icysock: The network subsystem has failed\n");
        break;

      case WSAEINPROGRESS:
        fprintf(
          stderr,
          "icysock: A blocking Windows Sockets 1.1 call is in progress, or the "
          "service provider is still processing a callback function. \n");
        break;

      default:
        fprintf(stderr, "icysock: WSACleanup() - Unknown error.\n");
    }
    std::terminate();
  }
#endif
}

}