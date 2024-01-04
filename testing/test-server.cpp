#include "socket/error_utils.hpp"
#include "socket/generic_sockets.hpp"
#include "socket/sockets.hpp"

#include <array>
#include <cstdio>

#ifndef ICY_ON_WINDOWS
#include <string.h>
#define ZeroMemory(ptr, n) memset(ptr, 0, n);
#endif

#define PORT "6900"
#define BUFLEN 512

int
main()
{
  icysock::init();
  BetterSockets::socket_hint hint(BetterSockets::ip_version::IpvAny,
                                  BetterSockets::sock_kind::STREAM,
                                  BetterSockets::sock_flags::USE_HOST_IP,
                                  BetterSockets::ip_protocol::TCP);
  BetterSockets::managed_socket ListenSock(hint, PORT);
  ListenSock.binds();
  ListenSock.listens();

  BetterSockets::managed_socket ClientSock(ListenSock.accepts());
  icysock::ssize iResult;
  icysock::ssize iSendResult;

  std::array<char, BUFLEN> recvbuf = { 0 };
  // Receive until the peer shuts down the connection
  do {

    iResult = ClientSock.receive(recvbuf.data(), recvbuf.size());
    if (iResult > 0) {
      fprintf(stdout, "Bytes received: %ld\n", iResult);
      fprintf(stdout, "Contents: %s\n", recvbuf.data());
      iSendResult = ClientSock.sends(recvbuf.data());
      fprintf(stdout, "Bytes sent: %ld\n", iSendResult);
    } else if (iResult == 0)
      fprintf(stdout, "Connection closing...\n");

  } while (iResult > 0);

  return 0;
}