#ifndef ICETEA_SOCKETS_H
#define ICETEA_SOCKETS_H

#include <iterator>
#include <string_view>

#include "generic_sockets.hpp"

namespace BetterSocket {

enum class TransmissionEnd
{
  NoRecv = SHUT_RD,
  NoSend = SHUT_WR,
  Everything = SHUT_RDWR,
};

/* Socket abstraction */
enum class IpVersion
{
  v4 = AF_INET,
  v6 = AF_INET6,
  vAny = AF_UNSPEC,
};

enum class SockKind
{
  Stream = SOCK_STREAM,
  Datagram = SOCK_DGRAM,
};

enum class SockFlags
{
  None = -1,
  UseHostIP = AI_PASSIVE,
};

enum class IpProtocol
{
  UDP = IPPROTO_UDP,
  TCP = IPPROTO_TCP,
};

/* structure to be used for the construction of
 * `bsocket` class
 */
struct SocketHint
{
  enum IpVersion hostIpVersion;
  enum SockKind socket_kind;
  enum SockFlags flags;
  enum IpProtocol ipproto;
  SocketHint(const IpVersion v,
             const SockKind k,
             const SockFlags f,
             const IpProtocol ipproto);
};

/* wrapper around `struct addrinfo` with the following features:
 * - Iterator implementation to loop over the list
 * - Equality Comparison operators for Iterator compatibilty
 */
struct AddressinfoHandle
{
private:
  struct addrinfo* beginP;
  struct addrinfo* endP;

public:
  struct addrinfo* infoP;

  AddressinfoHandle();
  AddressinfoHandle(const std::string hostname,
                    const std::string service,
                    const struct SocketHint hint);
  AddressinfoHandle(const AddressinfoHandle& h);
  ~AddressinfoHandle();

  struct Iterator
  {
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = struct addrinfo;
    using pointer = struct addrinfo*;
    using reference = struct addrinfo&;

    Iterator(pointer iptr);

    reference operator*() const;
    pointer operator->();

    Iterator& operator++();
    Iterator operator++(int); // postfix increment

    friend bool operator==(const Iterator& lhs, const Iterator& rhs);
    friend bool operator!=(const Iterator& lhs, const Iterator& rhs);

  private:
    pointer addrinfoPtr;
  }; // struct Iterator

  Iterator begin();
  Iterator end();

  /* only make sure that the list begin, end pointers are the same*/
  friend bool operator==(const AddressinfoHandle& lhs,
                         const AddressinfoHandle& rhs);
  friend bool operator!=(const AddressinfoHandle& lhs,
                         const AddressinfoHandle& rhs);

  void next();

}; // struct AddressinfoHandle

/* Managed class that wraps over the C API.
 * Not every function is wrapped over, only the handful ones that need
 * be used in avantee. They are as follows:
 * Return Type       Identifier    Params                 Wrapping-Over
 * void              binds         bool                   bind
 * void              connects      [None]                 connect
 * void              listers       [None]                 listen
 * BetterSocket::ssize    receieve      void*,                 recv
 *                                 BetterSocket::size,
 *                                 int (default)
 * BetterSocket::ssize    recieve_from  void*,                 recvfrom
 *                                 BetterSocket::size,
 *                                 struct sockaddr*
 *                                 BetterSocket::size*
 *                                 int (default)
 * BetterSocket::ssize   sends          std::string_view,      send
 *                                 int (default)
 * void             shutdowns      enum TransmissionEnd   shutdown
 * void             try_next       [None]                 [None]
 */
class BSocket
{
  bool bindsCalled{ false };
  bool empty{ true };
  bool IsListener{ false };
  AddressinfoHandle addressinfoList = {};

  void initRawSocket(struct addrinfo* a);

public:
  struct addrinfo validAddr = {};
  BetterSocket::gsocket rawSocket;
  BSocket();
  BSocket(BetterSocket::gsocket s);
  BSocket(BSocket&& ms);
  BSocket(const struct SocketHint hint,
          const std::string service,
          const std::string hostname = "");

  ~BSocket();
  bool IsEmpty() const;
  BetterSocket::gsocket underlyingSocket() const;
  friend bool operator==(const int& lhs, const BSocket& rhs);
  friend bool operator!=(const int& lhs, const BSocket& rhs);
  friend bool operator==(const BSocket& lhs, const int& rhs);
  friend bool operator!=(const BSocket& lhs, const int& rhs);
  friend bool operator==(const BSocket& lhs, const BSocket& rhs);
  friend bool operator!=(const BSocket& lhs, const BSocket& rhs);

  // `man 2 accept` takes 3 arguments, two of
  // which will contain relevant information
  // about the peer connection. Currently I
  // have no abstraction for the `struct
  // sockaddr*` structure therefore we will
  // NULL those arguments by defualt. As soon
  // as I have a proper thought out solution, I
  // shall implement it.
  [[nodiscard("Accepted socket must be used.")]] BetterSocket::gsocket
  acceptS();
  void bindS(bool reuseSocket = true);
  void connectS();
  void listenS(); // This will call binds() for you. This is because normally
                  // the accept()'ing socket needs to be "bound" to some socket
                  // addr.
  BetterSocket::ssize receive(void* ibuf, BetterSocket::size s, int flags = 0);
  BetterSocket::ssize receiveFrom(void* ibuf,
                                  BetterSocket::size bufsz,
                                  struct sockaddr* senderAddr,
                                  BetterSocket::size* sndrsz,
                                  int flags = 0);
  BetterSocket::ssize sendS(std::string_view buf, int flags = 0);
  BetterSocket::ssize sendTo(void* ibuf,
                             BetterSocket::size bufsz,
                             struct sockaddr* destAddr,
                             BetterSocket::size destsz,
                             int flags = 0);
  void shutdownS(enum TransmissionEnd reason);
  void tryNext();

}; // class BSocket

/* Abstraction for fd_set */

enum class FdType
{
  Read,
  Write,
  Exception,
};

template<FdType f>
struct FdsetWrapper
{
  fd_set set;
  FdType type;

  FdsetWrapper();
  FdsetWrapper(fd_set s);
  FdsetWrapper(FdsetWrapper& s);
  FdsetWrapper(FdsetWrapper&& s);

  void append(BetterSocket::gsocket s);
  void append(BSocket& s);
  void emptyOut();
  int isSet(BetterSocket::gsocket s);
  int isSet(BSocket& s);

  FdsetWrapper& operator=(FdsetWrapper& rhs);
};

/* Implementation of fd_set wrapper */

template<BetterSocket::FdType f>
inline BetterSocket::FdsetWrapper<f>::FdsetWrapper()
  : set{}
  , type{ f }
{
  FD_ZERO(&set);
}

template<BetterSocket::FdType f>
inline BetterSocket::FdsetWrapper<f>::FdsetWrapper(fd_set s)
  : set{ s }
  , type{ f }
{
}

template<BetterSocket::FdType f>
inline BetterSocket::FdsetWrapper<f>::FdsetWrapper(FdsetWrapper<f>& s)
  : set{ s.set }
  , type{ s.type }
{
}

template<BetterSocket::FdType f>
inline BetterSocket::FdsetWrapper<f>::FdsetWrapper(FdsetWrapper<f>&& s)
  : set{ s.set }
  , type{ s.type }
{
  s.set = nullptr;
  s.type = FdType::Exception;
  this->emptyOut();
}

template<BetterSocket::FdType f>
void inline BetterSocket::FdsetWrapper<f>::append(BetterSocket::gsocket s)
{
  FD_SET(s, &set);
}

template<BetterSocket::FdType f>
void inline BetterSocket::FdsetWrapper<f>::append(BSocket& s)
{
  FD_SET(s.underlyingSocket(), &set);
}

template<BetterSocket::FdType f>
inline void
BetterSocket::FdsetWrapper<f>::emptyOut()
{
  FD_ZERO(&set);
}

template<BetterSocket::FdType f>
inline int
BetterSocket::FdsetWrapper<f>::isSet(BetterSocket::gsocket s)
{
  return FD_ISSET(s, &set);
}

template<BetterSocket::FdType f>
inline int
BetterSocket::FdsetWrapper<f>::isSet(BetterSocket::BSocket& s)
{
  return FD_ISSET(s.underlyingSocket(), &set);
}

template<FdType f>
inline FdsetWrapper<f>&
FdsetWrapper<f>::operator=(FdsetWrapper<f>& rhs)
{
  this->set = rhs.set;
  this->type = rhs.type;
  return *this;
}

// finish FdsetWrapper

} // namespace BetterSocket

#endif // ICETEA_SOCKETS_H
