#include <cstdio>
#include <cstring>

#include "socket/generic_sockets.hpp"

namespace BetterSocket {

Sockdata
init()
{
  int result = 0;
#if defined(ICY_ON_WINDOWS)
  Sockdata wsadata;
  result = WSAStartup(MAKEWORD(2, 2), &wsadata);
  if (result != 0) {
    fprintf(stderr, "BetterSocket: WSAStartup failed with code: %d\n", result);
    perror("BetterSocket: ");
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
  if (WSACleanup() == SOCK_ERR) {
    switch (WSAGetLastError()) {
      case WSANOTINITIALISED:
        fprintf(stderr,
                "BetterSocket: A successful WSAStartup call did not occur!\n");
        break;

      case WSAENETDOWN:
        fprintf(stderr, "BetterSocket: The network subsystem has failed\n");
        break;

      case WSAEINPROGRESS:
        fprintf(stderr,
                "BetterSocket: A blocking Windows Sockets 1.1 call is in "
                "progress, or the "
                "service provider is still processing a callback function. \n");
        break;

      default:
        fprintf(stderr, "BetterSocket: WSACleanup() - Unknown error.\n");
    }
  }
#endif
}

int
closeSocket(GSocket s)
{
#if defined(ICY_ON_WINDOWS)
  return closesocket(s);
#else
  return close(s);
#endif
}

int
gPoll(GPollfd* fds, Size fdcnt, int timeout)
{
#if defined(ICY_ON_WINDOWS)
  return WSAPoll(fds, fdcnt, timeout);
#else
  return poll(fds, fdcnt, timeout);
#endif
}

void*
zero(void* p, Size len)
{
#if defined(ICY_ON_WINDOWS)
  ZeroMemory(p, len);
  return p;
#else
  return memset(p, 0, len);
#endif
}
}
