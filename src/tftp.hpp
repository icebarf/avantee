#ifndef AVANTEE_TFTP_H
#define AVANTEE_TFTP_H

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <utility>

#include "socket/socket.hpp"

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

struct RequestPacket
{
  int16_t opcode;
  char filename[std::to_underlying(Constants::MaxFilenameLen)];
  char mode[std::to_underlying(Constants::MaxModeStringLen)];
  void* data();
  BetterSocket::Size size();
} __attribute((packed));

struct DataPacket
{
  int16_t opcode;
  int16_t block;
  std::array<std::byte, std::to_underlying(Constants::MaxDataLen)> content;
  void* data();
  BetterSocket::Size size();
} __attribute((packed));

struct AckPacket
{
  int16_t opcode;
  int16_t block;
  void* data();
  BetterSocket::Size size();
} __attribute((packed));

struct ErrorPacket
{
  int16_t opcode;
  int16_t error_code;
  char error_msg[std::to_underlying(Constants::MaxErrorMsgLen)];
  void* data();
  BetterSocket::Size size();
} __attribute((packed));

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
  int16_t opcode;
  std::byte rawData[largestPacketSize() - sizeof(opcode)];
  void* data();
  BetterSocket::Size size();
} __attribute((packed));

struct Connection
{
  BetterSocket::BSocket peer;
  std::string filename;
  BetterSocket::Size block;
};

#endif
