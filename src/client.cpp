#include "socket/socket.hpp"

namespace BS=BetterSocket;

int
main(int argc, char** argv)
{
  if (argc < 2) {
    printf("./avantee-client IP\n");
    return 1;
  }
  BS::init();
  BS::SocketHint hint(BetterSocket::IpVersion::vAny,
                                 BetterSocket::SockKind::Datagram,
                                 BetterSocket::SockFlags::UseHostIP,
                                 BetterSocket::IpProtocol::UDP);
  BS::BSocket tftp(hint, "69", argv[1]); // tftp port: 69
  
  const char msg[] = "hi there avantee server, this is clientee :D";
  BS::SockaddrWrapper to(tftp.getsockaddr());
  long out = tftp.sendTo((void*) msg, sizeof(msg), to);
  
  printf("client.cpp: sent %ld bytes\n", out);

}
