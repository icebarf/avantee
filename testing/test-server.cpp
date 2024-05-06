#include "socket/generic_sockets.hpp"
#include "socket/socket.hpp"

#include <array>
#include <cstdio>

#define PORT "6900"
#define BUFLEN 512

int
main()
{
  BetterSocket::init();
  BetterSocket::SocketHint hint(BetterSocket::IpVersion::vAny,
				BetterSocket::SockKind::Stream,
				BetterSocket::SockFlags::UseHostIP,
                                  BetterSocket::IpProtocol::TCP);
  BetterSocket::BSocket ListenSock(hint, PORT);
  ListenSock.bindS();
  ListenSock.listenS();

  BetterSocket::BSocket ClientSock(ListenSock.acceptS());
  BetterSocket::ssize iResult;
  BetterSocket::ssize iSendResult;

  std::array<char, BUFLEN> recvbuf = { 0 };
  // Receive until the peer shuts down the connection
  do {

    iResult = ClientSock.receive(recvbuf.data(), recvbuf.size());
    if (iResult > 0) {
      fprintf(stdout, "Bytes received: %ld\n", iResult);
      fprintf(stdout, "Contents: %s\n", recvbuf.data());
      iSendResult = ClientSock.sendS(recvbuf.data());
      fprintf(stdout, "Bytes sent: %ld\n", iSendResult);
    } else if (iResult == 0)
      fprintf(stdout, "Connection closing...\n");

  } while (iResult > 0);

  return 0;
}
