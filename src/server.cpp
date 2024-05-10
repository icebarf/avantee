#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <string>
#include <utility>

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

enum class Constants : unsigned long
{
  MaxDataLen = 512,
  MaxFilenameLen = 255,
  MaxModeStringLen = sizeof("netascii"),
  MaxErrorMsgLen = 255,
};

struct request_packet
{
  int16_t opcode;
  char filename[std::to_underlying(Constants::MaxFilenameLen)];
  char mode[std::to_underlying(Constants::MaxModeStringLen)];
} __attribute((packed));

struct data_packet
{
  int16_t opcode;
  int16_t block;
  std::array<std::byte, std::to_underlying(Constants::MaxDataLen)> data;
} __attribute((packed));

struct ack_packet
{
  int16_t opcode;
  int16_t block;
} __attribute((packed));

struct error_packet
{
  int16_t opcode;
  int16_t error_code;
  char error_msg[std::to_underlying(Constants::MaxErrorMsgLen)];
} __attribute((packed));

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

struct __attribute((packed)) GenericPacket {
  int16_t opcode;
  std::byte __padding[largest_packet_size() - sizeof(opcode)]; 
};

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
