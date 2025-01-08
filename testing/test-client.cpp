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

  if (argc < 2) {
    fprintf(stderr, "Usage: ./client ipaddr\n");
    return 1;
  }

  BetterSocket::SocketHint h(BetterSocket::IpVersion::v4,
                             BetterSocket::SockKind::Stream,
                             BetterSocket::SockFlags::UseHostIP,
                             BetterSocket::IpProtocol::TCP);

  BetterSocket::BSocket sock(h, PORT, argv[1]);

  try {
    sock.connect();
  } catch (const SockErrors::APIError& e) {
    printf("Error: %s\n", e.what());
  }
  const char* sendbuf = "this is a test";
  BetterSocket::SSize iResult = sock.send(sendbuf, 0);
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
