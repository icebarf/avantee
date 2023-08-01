#ifndef ICETEA_SOCKETS_H
#define ICETEA_SOCKETS_H

#include <iterator>
#include <netinet/in.h>
#include <string_view>

#include "generic_sockets.hpp"

namespace BetterSockets {

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
  USE_HOST_IP = AI_PASSIVE,
};

enum class ip_protocol
{
  UDP = IPPROTO_UDP,
  TCP = IPPROTO_TCP,
};

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

/* wrapper around addrinfo with iterators */
struct addressinfo_handle
{
private:
  struct addrinfo* end_p;

public:
  struct addrinfo* info;

  addressinfo_handle();
  addressinfo_handle(const std::string_view hostname,
                     const std::string_view service,
                     const struct socket_hint hint);
  ~addressinfo_handle();

  addressinfo_handle& operator=(void* info_v);

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
  struct addressinfo_handle addressinfolist;

public:
  managed_socket();
  managed_socket(icysock::icy_socket s);
  managed_socket(managed_socket&& ms);
  managed_socket(const std::string_view hostname,
                 const std::string_view service,
                 const struct socket_hint hint);

}; // class ManagedSocket

} // namespace BetterSockets

#endif // ICETEA_SOCKETS_H
