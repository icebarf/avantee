#include <cstdio>
#include <cstring>
#include <exception>

#include "socket/generic_sockets.hpp"

namespace icysock {

sockdata
init()
{
  int result = 0;
#if defined(ICY_ON_WINDOWS)
  SOCKDATA wsadata;
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
  if (WSACleanup() == SOCK_ERR) {
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
  }
#endif
}

int
close_socket(gsocket s)
{
#if defined(ICY_ON_WINDOWS)
  return closesocket(s);
#else
  return close(s);
#endif
}

void*
zero(void* p, size len)
{
#if defined(ICY_ON_WINDOWS)
  ZeroMemory(p, len);
  return p;
#else
  return memset(p, 0, len);
#endif
}

}
