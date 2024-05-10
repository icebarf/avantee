#include <cstdio>
#include <string>

#include "multiplexer.hpp"
#include "socket/socket.hpp"
#include "tftp.hpp"

#define SCAST(Type, e) static_cast<Type>(e)
#define RCAST(Type, e) reinterpret_cast<Type>(e)

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

  for (;;) {
    multiplexer.poll_io();

    if (multiplexer.socket_available_for<Multiplexer::Events::INPUT>(
          tftp_listener.underlyingSocket())) {

      auto senderInfo = BS::SockaddrWrapper();
      if (tftp_listener.receiveFrom(packet.data(), packet.size(), senderInfo) !=
          1) {

        // print sender info
        printf("Sender Port: %d\nSender IP: %s\nPacket Content: %s\n",
               senderInfo.getPort(),
               senderInfo.getIP().data(),
               (const char*)packet.data()); // ub but we dont care right now
      }
    }
  }
}
