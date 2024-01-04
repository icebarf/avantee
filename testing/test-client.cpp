#include "socket/error_utils.hpp"
#include "socket/generic_sockets.hpp"
#include "socket/sockets.hpp"

#include <array>
#include <cstdio>

#define PORT "6900"
#define BUFLEN 512

int
main(int argc, char** argv)
{
  icysock::init(); // optional: only needed on windows.
  (void)argc;
  BetterSockets::socket_hint h(BetterSockets::ip_version::IpvAny,
                               BetterSockets::sock_kind::STREAM,
                               BetterSockets::sock_flags::USE_HOST_IP,
                               BetterSockets::ip_protocol::TCP);
  BetterSockets::managed_socket sock(h, PORT, argv[1]);
  sock.connects();

  const char* sendbuf = "this is a test";
  icysock::ssize iResult = sock.sends(sendbuf, 0);
  printf("Bytes sent: %ld\n", iResult);

  std::array<char, BUFLEN> recvbuf{ 0 };
  do {
    try {
      iResult = sock.receive(recvbuf.data(), recvbuf.size(), 0);
    } catch (const icysock::errors::APIError& e) {
      fprintf(stderr, "%s\n", e.what());
      continue;
    };
    if (iResult > 0)
      fprintf(stdout, "Bytes receieved: %ld\n", iResult);
    else if (iResult == 0)
      fprintf(stdout, "Connection closed\n");

  } while (iResult > 0);

  return 0;
}