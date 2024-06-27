#ifndef AVANTEE_TFTP_H
#define AVANTEE_TFTP_H

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <variant>

#include "socket/socket.hpp"

#define TU(enum) std::to_underlying(enum)


enum class Opcodes : int16_t
{
  ack,
  data,
  error,
  rrq, // read-request
  wrq, // write-request
};

enum class Constants : unsigned long
{
  maxDataLen = 512,
  maxFilenameLen = 255,
  maxModeStringLen = sizeof("netascii"),
  maxErrorMsgLen = 255,
  maxConnections = 64,
  unprivPortsLower = 1025,
  unprivPortsUpper = 65535,
};

struct AckPacket
{
  Opcodes opcode;
  int16_t block;
  void* data();
  BetterSocket::Size size();
} __attribute((packed));

struct DataPacket
{
  Opcodes opcode;
  int16_t block;
  std::array<std::byte, TU(Constants::maxDataLen)> content;
  void* data();
  BetterSocket::Size size();
} __attribute((packed));


struct ErrorPacket
{
  Opcodes opcode;
  int16_t error_code;
  char error_msg[TU(Constants::maxErrorMsgLen)];
  void* data();
  BetterSocket::Size size();
} __attribute((packed));

struct RequestPacket
{
  Opcodes opcode;
  char filename[TU(Constants::maxFilenameLen)];
  char mode[TU(Constants::maxModeStringLen)];
  void* data();
  BetterSocket::Size size();
} __attribute((packed));

using PacketVariant=std::variant<AckPacket,DataPacket,ErrorPacket,RequestPacket>;

consteval BetterSocket::Size
largestPacketSize()
{

  return std::max(
    {
      sizeof(RequestPacket),
      sizeof(DataPacket),
      sizeof(AckPacket),
      sizeof(ErrorPacket),
    },
    [](const BetterSocket::Size e1, const BetterSocket::Size e2) {
      return e1 < e2;
    });
}

struct GenericPacket
{
  Opcodes opcode;
  std::byte rawData[largestPacketSize() - sizeof(opcode)];
  void* data();
  BetterSocket::Size size();
} __attribute((packed));

struct Connection
{
  BetterSocket::BSocket peer;
  PacketVariant prevPacket;
  PacketVariant curPacket;
  std::string associatedFile;
  int peerLocalPort;
  bool IsActive;
  bool IsBad;
};

// returns a random port between 1025 and 65,535 (the unprivleged ports)
BetterSocket::in_port_t
randomPort();

#undef TU

#endif
