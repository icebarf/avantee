#include "tftp.hpp"
#include "socket/generic_sockets.hpp"
#include <random>
#include <utility>
#include <variant>

#define TU(x) std::to_underlying(x)

void*
RequestPacket::data()
{
  return &this->opcode;
}

BetterSocket::Size
RequestPacket::size()
{
  return sizeof(TU(opcode)) + sizeof(filename) + sizeof(mode);
}

void*
DataPacket::data()
{
  return &this->opcode;
}

BetterSocket::Size
DataPacket::size()
{
  return sizeof(TU(opcode)) + sizeof(block) + sizeof(content);
}

void*
AckPacket::data()
{
  return &this->opcode;
}

BetterSocket::Size
AckPacket::size()
{
  return sizeof(TU(opcode)) + sizeof(block);
}

void*
ErrorPacket::data()
{
  return &this->opcode;
}

BetterSocket::Size
ErrorPacket::size()
{
  return sizeof(TU(opcode)) + sizeof(error_code) + sizeof(error_msg);
}

void*
GenericPacket::data()
{
  return &this->opcode;
}

BetterSocket::Size
GenericPacket::size()
{
  return sizeof(TU(opcode)) + sizeof(rawData);
}

BetterSocket::in_port_t
randomPort()
{
  std::random_device dv;
  std::default_random_engine engine(dv());
  std::uniform_int_distribution<BetterSocket::in_port_t> distribution{
    std::to_underlying(Constants::unprivPortsLower),
    std::to_underlying(Constants::unprivPortsUpper)
  };

  return distribution(engine);
}
