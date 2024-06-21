#include "socket/socket.hpp"
#include <iostream>
#include <random>

using namespace BetterSocket;

in_port_t randomPort() {
  std::random_device dev;
  std::default_random_engine engine(std::move(dev()));
  std::uniform_int_distribution<in_port_t> dist{1025, 65535};

  return dist(engine);
}

int
main()
{
  SocketHint hint(
    IpVersion::vAny, SockKind::Datagram, SockFlags::UseHostIP, IpProtocol::UDP);
  in_port_t port = randomPort();
  
  BSocket socket (hint, std::to_string(port));
  socket.bindS();

  SockaddrWrapper sockaddr(*socket.validAddr.ai_addr);
  std::cout << sockaddr.getIP() << ':' << sockaddr.getPort() << '\n';

  while(true);
}
