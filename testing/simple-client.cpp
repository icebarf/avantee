#include "socket/error_utils.hpp"
#include <cstdio>
#include <socket/socket.hpp>
#include <sys/socket.h>

using namespace BetterSocket;

#define PORT "3490" // the port client will be connecting to

#define MAXDATASIZE 100 // max number of bytes we can get at once

// get sockaddr, IPv4 or IPv6:
void*
get_in_addr(struct sockaddr* sa)
{
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int
main(int argc, char* argv[])
{
  int numbytes;
  char buf[MAXDATASIZE];

  if (argc != 2) {
    fprintf(stderr, "usage: client hostname\n");
    exit(1);
  }

  SocketHint hint(
    IpVersion::vAny, SockKind::Stream, SockFlags::None, IpProtocol::TCP);
  BSocket sockfd(hint, PORT, argv[1]);

  try {
    sockfd.connect();
  } catch (const SockErrors::APIError& e) {
    fprintf(stderr, "Client: %s", e.what());
    return 2;
  }

  SockaddrWrapper server(*sockfd.validAddr.ai_addr);
  printf("client: connecting to %s\n", server.getIP().c_str());

  if ((numbytes = sockfd.receive(buf, MAXDATASIZE)) == -1) {
    perror("recv");
    exit(1);
  }

  buf[numbytes] = '\0';

  printf("client: received '%s'\n", buf);

  return 0;
}