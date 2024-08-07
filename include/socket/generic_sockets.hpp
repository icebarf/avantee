#ifndef SOCKET_ABSTACTION_H
#define SOCKET_ABSTACTION_H

/* The goal of this thin wrapper is to not bother with lower level C
 * api. Things known to be different on platform such as windows -
 * types, functions are aliased or wrapped upon, and other values may be
 * specified as preprocessor macros
 */

/* neatly avoid checking this everywhere */
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) ||                   \
  defined(__WIN32__) || defined(WIN64) || defined(_WIN64) ||                   \
  defined(__WIN64) || defined(__WIN64__)
#define ICY_ON_WINDOWS
#endif

#if defined(ICY_ON_WINDOWS)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

namespace BetterSocket {

#define BAD_SOCKET INVALID_SOCKET
#define SOCK_ERR SOCKET_ERROR
#define SHUT_RD SD_RECEIVE
#define SHUT_WR SD_SEND
#define SHUT_RDWR SD_BOTH

/* introduce alias types on windows */
using Sockdata = WSADATA;
using GSocket = SOCKET;

using GPollfd = WSAPOLLFD;
using in_port_t = u_short;
}

#else

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace BetterSocket {

#define BAD_SOCKET -1
#define SOCK_ERR -1

/* introduce alias types on other platform */
using Sockdata = int;
using GSocket = int;

using GPollfd = struct pollfd;
using in_port_t = in_port_t;
}

#endif

#include <cstdint>

namespace BetterSocket {

/* Signed return type for size */
using SSize = std::intmax_t;
using Size = std::uintmax_t;

/* initialise the sockets library
 * this is only needed on windows but this should be called
 * before using this abstraction on a platform as it may setup
 * additional structures and variables not part of the windows
 * API
 */
[[maybe_unused]] Sockdata
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
closeSocket(GSocket s);

/* Wrapper over `poll()`*/
int
gPoll(GPollfd* fds, Size fdcnt, int timeout);

/* Zero out the memory in range [p+0, p+len) */
void*
zero(void* p, Size len);

}
#endif
