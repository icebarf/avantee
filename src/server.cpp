#include <cstdio>
#include <netinet/in.h>
#include <string>

#include "multiplexer.hpp"
#include "socket/socket.hpp"
#include "tftp.hpp"

#define SCAST(Type, e) static_cast<Type>(e)
#define RCAST(Type, e) reinterpret_cast<Type>(e)
#define TU(enum) std::to_underlying(enum)

namespace BS = BetterSocket;

int
main()
{
  BS::init();
  BS::SocketHint hint(BS::IpVersion::vAny,
                      BS::SockKind::Datagram,
                      BS::SockFlags::UseHostIP,
                      BS::IpProtocol::UDP);

  BS::BSocket tftp_listener(hint, "69"); // tftp port: 69
  tftp_listener.bindS();

  Multiplexer multiplexer;
  multiplexer.watch(tftp_listener.underlyingSocket(),
                    Multiplexer::Events::INPUT);

  GenericPacket packet;
  BS::zero(packet.data(), packet.size());

  std::array<Connection,
             std::to_underlying(Multiplexer::constants::MAX_SERVER_CONNECTIONS)>
    regCon;
  BS::zero(regCon.data(),
           sizeof(Connection) *
             SCAST(BS::Size, Multiplexer::constants::MAX_SERVER_CONNECTIONS));
  BS::Size curCon = 0;

  for (;;) {
    multiplexer.poll_io();

    if (multiplexer.socket_available_for<Multiplexer::Events::INPUT>(
          tftp_listener.underlyingSocket())) {

      // new connection
      auto senderInfo = BS::SockaddrWrapper();
      if (tftp_listener.receiveFrom(packet.data(), packet.size(), senderInfo) !=
          1) {
        int port = randomPort();
        regCon[curCon].peer = BS::BSocket(hint, std::to_string(port));
        regCon[curCon].peerLocalPort = port;
        regCon[curCon].curPacket = packet;
        regCon[curCon].IsActive = true;

      } // new connection created
    }
  }
}
