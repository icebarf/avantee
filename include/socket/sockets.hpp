#ifndef ICETEA_SOCKETS_H
#define ICETEA_SOCKETS_H

#include <iterator>
#include <string_view>

#include "generic_sockets.hpp"

namespace BetterSockets {

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
  addressinfo_handle(const std::string hostname,
                     const std::string service,
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
  bool binds_called{ false };
  bool empty{ true };
  bool is_listener{ false };
  icysock::gsocket socket_handle{ BAD_SOCKET };
  addressinfo_handle addressinfolist = {};
  struct addrinfo valid_addr = {};

public:
  managed_socket();
  managed_socket(icysock::gsocket s);
  managed_socket(managed_socket&& ms);
  managed_socket(const struct socket_hint hint,
                 const std::string service,
                 const std::string hostname = "");

  ~managed_socket();
  bool is_empty();

  // `man 2 accept` takes 3 arguments, two of
  // which will contain relevant information
  // about the peer connection. Currently I
  // have no abstraction for the `struct
  // sockaddr*` structure therefore we will
  // NULL those arguments by defualt. As soon
  // as I have a proper thought out solution, I
  // shall implement it.
  [[nodiscard("Accepted socket must be used.")]] icysock::gsocket accepts();
  void binds(bool reuse_socket = true);
  void connects();
  void
  listens(); // This will call binds() for you. This is because normally
             // the ccept()'ing socket needs to be "bound" to some socket addr.
  icysock::ssize receive(char* buf, icysock::ssize s, int flags = 0);
  icysock::ssize sends(std::string_view buf, int flags = 0);
  void shutdowns(enum TransmissionEnd reason);

}; // class ManagedSocket

} // namespace BetterSockets

#endif // ICETEA_SOCKETS_H
