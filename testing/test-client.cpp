#include "socket/error_utils.hpp"
#include "socket/generic_sockets.hpp"
#include "socket/socket.hpp"

#include <array>
#include <cstdio>

#define PORT "6900"
#define BUFLEN 512

int
main(int argc, char** argv)
{
  BetterSocket::init(); // optional: only needed on windows.
  (void)argc;
  BetterSocket::SocketHint h(BetterSocket::IpVersion::vAny,
                               BetterSocket::SockKind::Stream,
			     BetterSocket::SockFlags::UseHostIP,
                               BetterSocket::IpProtocol::TCP);
  BetterSocket::BSocket sock(h, PORT, argv[1]);
  sock.connectS();

  const char* sendbuf = "this is a test";
  BetterSocket::ssize iResult = sock.sendS(sendbuf, 0);
  printf("Bytes sent: %ld\n", iResult);

  std::array<char, BUFLEN> recvbuf{ 0 };
  do {
    try {
      iResult = sock.receive(recvbuf.data(), recvbuf.size(), 0);
    } catch (const SockErrors::APIError& e) {
      fprintf(stderr, "%s\n", e.what());
      continue;
    };
    if (iResult > 0)
      fprintf(stdout, "Bytes receieved: %ld\n%s\n", iResult, recvbuf.data());
    else if (iResult == 0)
      fprintf(stdout, "Connection closed\n");

  } while (iResult > 0);

  return 0;
}
