#include <cstdio>
#include <string>
#include <variant>

#include "multiplexer.hpp"
#include "socket/socket.hpp"
#include "tftp.hpp"

#define SCAST(Type, e) static_cast<Type>(e)
#define RCAST(Type, e) reinterpret_cast<Type>(e)
#define TU(enum) std::to_underlying(enum)

namespace BS = BetterSocket;

// unreadable mess only once
using ConnectionsType =
  std::array<Connection,
             TU(Multiplexer::constants::MAX_SERVER_CONNECTIONS) + 1>;

Connection&
findInactive(ConnectionsType& c)
{
  for (auto& con : c) {
    if (con.IsActive == false && con.IsBad == false)
      return con;
  }

  return c[TU(Multiplexer::constants::MAX_SERVER_CONNECTIONS)];
}

void
registerClient(ConnectionsType& connections,
               GenericPacket& packet,
               const auto& hint)
{
  int port = randomPort(); // for this connection's server-side TID

  auto& connection = findInactive(connections);
  connection.peer = BS::BSocket(hint, std::to_string(port));
  connection.peerLocalPort = port;

  connection.curPacket = [&]() -> PacketVariant {
    auto opcode = packet.opcode;
    switch (opcode) {
      case Opcodes::ack:
        return *reinterpret_cast<AckPacket*>(&packet);
      case Opcodes::data:
        return *reinterpret_cast<DataPacket*>(&packet);
      case Opcodes::error:
        return *reinterpret_cast<ErrorPacket*>(&packet);
      case Opcodes::rrq:
      case Opcodes::wrq:
        return *reinterpret_cast<RequestPacket*>(&packet);
    }
  }();

  connection.IsActive = true;
}

void
runConnections(ConnectionsType& connections, const auto& hint)
{
  // parse the packet and then perform operations
  for (auto& con : connections) {
    if (std::holds_alternative<AckPacket>(con.curPacket)) {
      auto& actualPacket = std::get<AckPacket>(con.curPacket);
    }
  }
}

int
main()
{
  BS::init();
  BS::SocketHint hint(BS::IpVersion::vAny,
                      BS::SockKind::Datagram,
                      BS::SockFlags::UseHostIP,
                      BS::IpProtocol::UDP);

  BS::BSocket tftp_listener(hint, "69"); // tftp port: 69
  tftp_listener.bind();

  Multiplexer multiplexer;
  multiplexer.watch(tftp_listener.underlyingSocket(),
                    Multiplexer::Events::input);

  GenericPacket packet;

  ConnectionsType connections;
  BS::zero(connections.data(),
           sizeof(Connection) *
             SCAST(BS::Size, Multiplexer::constants::MAX_SERVER_CONNECTIONS));
  connections[TU(Constants::maxConnections)].IsBad =
    true; // mark the last connection as bad. This is done for error checking
          // when maximum connections are reached.

  for (;;) {
    multiplexer.poll_io();

    if (multiplexer.socket_available_for<Multiplexer::Events::input>(
          tftp_listener.underlyingSocket())) {
      // new connection
      auto senderInfo = BS::SockaddrWrapper();
      if (tftp_listener.receiveFrom(packet.data(), packet.size(), senderInfo) !=
          1) {
        registerClient(connections, packet, hint);
      } // new connection created
    }

    runConnections(connections, hint);

    // go through each connection and we try and work on what they do
  }
}
