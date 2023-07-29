#ifndef ICETEA_SOCKETS_H
#define ICETEA_SOCKETS_H

#include <exception>
#include <iterator>
#include <string>
#include <string_view>

#include "generic_sockets.hpp"

namespace BetterSockets {

/* Exceptions */

class SocketInitError : public std::exception
{
private:
  std::string what_string;

public:
  explicit SocketInitError(const std::string& what_arg);
  explicit SocketInitError(const char* what_arg);
  SocketInitError(const SocketInitError& other);

  SocketInitError& operator=(const SocketInitError& other) noexcept;

  const char* what() const noexcept override;
};

/* Socket abstraction */

enum ip_t
{
  IPv4 = AF_INET,
  IPv6 = AF_INET6,
  IpvAny = AF_UNSPEC,
};

/* stream sockets and datagram sockets aren't supposed
 * to be strictly TCP and UDP sockets respectively, but
 * we can just assume for our case
 */
enum sock_t
{
  TCP = SOCK_STREAM,
  UDP = SOCK_DGRAM,
};

enum flags_t
{
  HOST_IP = AI_PASSIVE,
};

struct socket_hint
{
  enum ip_t hostip_type;
  enum sock_t socket_type;
  enum flags_t flags;
  socket_hint(const ip_t i, const sock_t s, const flags_t f);
};

/* wrapper around addrinfo with iterators */
struct addressinfo_handle
{
private:
  struct addrinfo* end_p;

public:
  struct addrinfo* info;

  addressinfo_handle(const std::string_view hostname,
                     const std::string_view service,
                     const struct socket_hint hint);

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

}; // struct addressinfo_handle

/* wrap the shitty C api inside the methods and use it
 * im not that smart, but i think avoiding the primitives
 * for now is the way to go
 */
class managed_socket
{
  icysock::icy_socket socket_handle;
  struct addressinfo_handle addressinfo;

public:
  managed_socket(const std::string_view hostname,
                 const std::string_view service,
                 const struct socket_hint hint);

}; // class ManagedSocket

} // namespace BetterSockets

#endif // ICETEA_SOCKETS_H