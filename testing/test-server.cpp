#include "socket/generic_sockets.hpp"
#include "socket/sockets.hpp"

#include <array>
#include <cstdio>

#define PORT "6900"
#define BUFLEN 512

int
main()
{
  BetterSocket::init();
  BetterSocket::socket_hint hint(BetterSocket::ip_version::IpvAny,
                                  BetterSocket::sock_kind::STREAM,
                                  BetterSocket::sock_flags::USE_HOST_IP,
                                  BetterSocket::ip_protocol::TCP);
  BetterSocket::managed_socket ListenSock(hint, PORT);
  ListenSock.binds();
  ListenSock.listens();

  BetterSocket::managed_socket ClientSock(ListenSock.accepts());
  BetterSocket::ssize iResult;
  BetterSocket::ssize iSendResult;

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
