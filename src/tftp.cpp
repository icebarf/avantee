#include "tftp.hpp"
#include "socket/generic_sockets.hpp"

void*
RequestPacket::data()
{
  return &this->opcode;
}

BetterSocket::Size
RequestPacket::size()
{
  return sizeof(opcode) + sizeof(filename) + sizeof(mode);
}

void*
DataPacket::data()
{
  return &this->opcode;
}

BetterSocket::Size
DataPacket::size()
{
  return sizeof(opcode) + sizeof(block) + sizeof(content);
}

void*
AckPacket::data()
{
  return &this->opcode;
}

BetterSocket::Size
AckPacket::size()
{
  return sizeof(opcode) + sizeof(block);
}

void*
ErrorPacket::data()
{
  return &this->opcode;
}

BetterSocket::Size
ErrorPacket::size()
{
  return sizeof(opcode) + sizeof(error_code) + sizeof(error_msg);
}

void*
GenericPacket::data()
{
  return &this->opcode;
}

BetterSocket::Size
GenericPacket::size()
{
  return sizeof(opcode) + sizeof(rawData);
}
