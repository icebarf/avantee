#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <string>

#include "multiplexer.hpp"
#include "socket/socket.hpp"

#define SCAST(Type, e) static_cast<Type>(e)
#define RCAST(Type, e) reinterpret_cast<Type>(e)

enum class Opcodes : int16_t
{
  RRQ, // Read ReQuest
  WRQ, // Write ReQuest
  DATA,
  ACK,
  ERROR,
};

enum class constants
{
  MaxDataLen = 512,
};

struct request_packet
{
  int16_t opcode;
  std::string filename;
  std::string mode;
};

struct data_packet
{
  int16_t opcode;
  int16_t block;
  std::array<std::byte, SCAST(unsigned long, constants::MaxDataLen)> data;
};

struct ack_packet
{
  int16_t opcode;
  int16_t block;
};

struct error_packet
{
  int16_t opcode;
  int16_t error_code;
  std::string error_msg;
};

consteval BetterSocket::size
largest_packet_size()
{

  return std::max(
    {
      sizeof(request_packet),
      sizeof(data_packet),
      sizeof(ack_packet),
      sizeof(error_packet),
    },
    [](const BetterSocket::size e1, const BetterSocket::size e2) {
      return e1 < e2;
    });
}

int
main()
{
  BetterSocket::init();
  BetterSocket::SocketHint hint(BetterSocket::IpVersion::vAny,
                                BetterSocket::SockKind::Datagram,
                                BetterSocket::SockFlags::UseHostIP,
                                BetterSocket::IpProtocol::UDP);

  BetterSocket::BSocket tftp_listener(hint, "69"); // tftp port: 69
  tftp_listener.bindS();

  multiplexer multiplexer;
  multiplexer.watch(tftp_listener.underlyingSocket(),
                    multiplexer::events::INPUT);

  // allocate storage for atleast one packet for now
  std::array<std::byte, largest_packet_size()> packet;
  packet.fill(std::byte{ 0 });


  for (;;) {
    multiplexer.poll_io();

    if (multiplexer.socket_available_for<multiplexer::events::INPUT>(
          tftp_listener.underlyingSocket())) {
      
      auto senderInfo = BetterSocket::SockaddrWrapper();
      if (tftp_listener.receiveFrom(packet.data(), packet.size(), senderInfo) !=
          1) {

        // print sender info
        printf("Sender Port: %d\nSender IP: %s\nPacket Content: %s\n",
               senderInfo.getPort(),
               senderInfo.getIP().data(),
               (const char*)packet.data());
      }
    }
  }
}
