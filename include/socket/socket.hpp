#ifndef ICETEA_SOCKETS_H
#define ICETEA_SOCKETS_H

#include <iterator>
#include <string_view>

#include "generic_sockets.hpp"

namespace BetterSocket {

enum class TransmissionEnd
{
  NO_RECV = SHUT_RD,
  NO_TRANS = SHUT_WR,
  EVERYTHING = SHUT_RDWR,
};

/* Socket abstraction */
enum class ip_version
{
  IPv4 = AF_INET,
  IPv6 = AF_INET6,
  IpvAny = AF_UNSPEC,
};

enum class sock_kind
{
  STREAM = SOCK_STREAM,
  DGRAM = SOCK_DGRAM,
};

enum class sock_flags
{
  NONE = -1,
  USE_HOST_IP = AI_PASSIVE,
};

enum class ip_protocol
{
  UDP = IPPROTO_UDP,
  TCP = IPPROTO_TCP,
};

/* structure to be used for the construction of
 * `managed_socket` class
 */
struct socket_hint
{
  enum ip_version hostip_version;
  enum sock_kind socket_kind;
  enum sock_flags flags;
  enum ip_protocol ipproto;
  socket_hint(const ip_version v,
              const sock_kind k,
              const sock_flags f,
              const ip_protocol ipproto);
};

/* wrapper around `struct addrinfo` with the following features:
 * - Iterator implementation to loop over the list
 * - Equality Comparison operators for Iterator compatibilty
 */
struct addressinfo_handle
{
private:
  struct addrinfo* begin_p;
  struct addrinfo* end_p;

public:
  struct addrinfo* info;

  addressinfo_handle();
  addressinfo_handle(const std::string hostname,
                     const std::string service,
                     const struct socket_hint hint);
  addressinfo_handle(const addressinfo_handle& h);
  ~addressinfo_handle();

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
    pointer addrinfo_ptr;
  }; // struct Iterator

  Iterator begin();
  Iterator end();

  /* only make sure that the list begin, end pointers are the same*/
  friend bool operator==(const addressinfo_handle& lhs,
                         const addressinfo_handle& rhs);
  friend bool operator!=(const addressinfo_handle& lhs,
                         const addressinfo_handle& rhs);

  void next();

}; // struct addressinfo_handle

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
class managed_socket
{
  bool binds_called{ false };
  bool empty{ true };
  bool is_listener{ false };
  addressinfo_handle addressinfolist = {};

  void init_socket_handle(struct addrinfo* a);

public:
  struct addrinfo valid_addr = {};
  BetterSocket::gsocket socket_handle{ BAD_SOCKET };
  managed_socket();
  managed_socket(BetterSocket::gsocket s);
  managed_socket(managed_socket&& ms);
  managed_socket(const struct socket_hint hint,
                 const std::string service,
                 const std::string hostname = "");

  ~managed_socket();
  bool is_empty();
  friend bool operator==(const int& lhs, const managed_socket& rhs);
  friend bool operator!=(const int& lhs, const managed_socket& rhs);
  friend bool operator==(const managed_socket& lhs, const int& rhs);
  friend bool operator!=(const managed_socket& lhs, const int& rhs);
  friend bool operator==(const managed_socket& lhs, const managed_socket& rhs);
  friend bool operator!=(const managed_socket& lhs, const managed_socket& rhs);

  // `man 2 accept` takes 3 arguments, two of
  // which will contain relevant information
  // about the peer connection. Currently I
  // have no abstraction for the `struct
  // sockaddr*` structure therefore we will
  // NULL those arguments by defualt. As soon
  // as I have a proper thought out solution, I
  // shall implement it.
  [[nodiscard("Accepted socket must be used.")]] BetterSocket::gsocket accepts();
  void binds(bool reuse_socket = true);
  void connects();
  void listens(); // This will call binds() for you. This is because normally
                  // the accept()'ing socket needs to be "bound" to some socket
                  // addr.
  BetterSocket::ssize receive(void* ibuf, BetterSocket::size s, int flags = 0);
  BetterSocket::ssize receive_from(void* ibuf,
                              BetterSocket::size bufsz,
                              struct sockaddr* sender_addr,
                              BetterSocket::size* sndrsz,
                              int flags = 0);
  BetterSocket::ssize sends(std::string_view buf, int flags = 0);
  BetterSocket::ssize send_to(void* ibuf,
			 BetterSocket::size bufsz,
			 struct sockaddr* dest_addr,
			 BetterSocket::size destsz,
			 int flags = 0);
  void shutdowns(enum TransmissionEnd reason);
  void try_next();

}; // class ManagedSocket

/* Abstraction for fd_set */

enum class fd_type
{
  READ,
  WRITE,
  EXCEPT,
};

template<fd_type f>
struct fdset_wrapper
{
  fd_set set;
  fd_type type;

  fdset_wrapper();
  fdset_wrapper(fd_set s);
  fdset_wrapper(fdset_wrapper& s);
  fdset_wrapper(fdset_wrapper&& s);
  void append(BetterSocket::gsocket s);

  void append(managed_socket& s);
  void empty_out();
  int isset(BetterSocket::gsocket s);
  int isset(managed_socket& s);

  fdset_wrapper& operator=(fdset_wrapper& rhs);
};

/* Implementation of fd_set wrapper */

template<BetterSocket::fd_type f>
inline BetterSocket::fdset_wrapper<f>::fdset_wrapper()
  : set{}
  , type{ f }
{
  FD_ZERO(&set);
}

template<BetterSocket::fd_type f>
inline BetterSocket::fdset_wrapper<f>::fdset_wrapper(fd_set s)
  : set{ s }
  , type{ f }
{
}

template<BetterSocket::fd_type f>
inline BetterSocket::fdset_wrapper<f>::fdset_wrapper(fdset_wrapper<f>& s)
  : set{ s.set }
  , type{ s.type }
{
}

template<BetterSocket::fd_type f>
inline BetterSocket::fdset_wrapper<f>::fdset_wrapper(fdset_wrapper<f>&& s)
  : set{ s.set }
  , type{ s.type }
{
  s.set = nullptr;
  s.type = fd_type::EXCEPT;
  this->empty_out();
}

template<BetterSocket::fd_type f>
void inline BetterSocket::fdset_wrapper<f>::append(BetterSocket::gsocket s)
{
  FD_SET(s, &set);
}

template<BetterSocket::fd_type f>
void inline BetterSocket::fdset_wrapper<f>::append(managed_socket& s)
{
  FD_SET(s.socket_handle, &set);
}

template<BetterSocket::fd_type f>
inline void
BetterSocket::fdset_wrapper<f>::empty_out()
{
  FD_ZERO(&set);
}

template<BetterSocket::fd_type f>
inline int
BetterSocket::fdset_wrapper<f>::isset(BetterSocket::gsocket s)
{
  return FD_ISSET(s, &set);
}

template<BetterSocket::fd_type f>
inline int
BetterSocket::fdset_wrapper<f>::isset(BetterSocket::managed_socket& s)
{
  return FD_ISSET(s.socket_handle, &set);
}

template<fd_type f>
inline fdset_wrapper<f>&
fdset_wrapper<f>::operator=(fdset_wrapper<f>& rhs)
{
  this->set = rhs.set;
  this->type = rhs.type;
  return *this;
}

// finish fdset_wrapper

} // namespace BetterSocket

#endif // ICETEA_SOCKETS_H
