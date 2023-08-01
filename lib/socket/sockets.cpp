#include <cerrno>
#include <cstring>

#include "socket/error_utils.hpp"
#include "socket/generic_sockets.hpp"
#include "socket/sockets.hpp"

using namespace std;

namespace BetterSockets {

/*** Socket abstraction implementation ***/

/* struct socket_hint */
socket_hint::socket_hint(const ip_version v,
                         const sock_kind k,
                         const sock_flags f,
                         const ip_protocol ipr)
  : hostip_version(v)
  , socket_kind(k)
  , flags(f)
  , ipproto(ipr)
{
}

/* struct adressinfo_handle */

addressinfo_handle::addressinfo_handle()
  : end_p(nullptr)
  , info(nullptr)
{
}

addressinfo_handle::addressinfo_handle(const std::string_view hostname,
                                       const std::string_view service,
                                       const socket_hint hint)
{
  struct addrinfo hints = {};
  hints.ai_family = static_cast<int>(hint.hostip_version);
  hints.ai_socktype = static_cast<int>(hint.socket_kind);
  hints.ai_flags = static_cast<int>(hint.flags);
  hints.ai_protocol = static_cast<int>(hint.ipproto);

  int rv = getaddrinfo(hostname.data(), service.data(), &hints, &info);
  if (rv != 0) {
    throw icysock_errors::SocketInitError(
      icysock_errors::errc::getaddrinfo_failure, gai_strerror(rv));
  }

  /* want the pointer to the last element of list */
  auto p = info;
  for (; p->ai_next != nullptr; p = p->ai_next)
    ;
  end_p = p;
}

addressinfo_handle::~addressinfo_handle()
{
  freeaddrinfo(info);
  info = nullptr;
}

addressinfo_handle&
addressinfo_handle::operator=(void* info_v)
{
  this->info = (struct addrinfo*)info_v;
  return *this;
}

/* addressinfo_handle Iterator implementation */

addressinfo_handle::Iterator::Iterator(
  addressinfo_handle::Iterator::pointer iptr)
  : addrinfo_ptr(iptr)
{
}

addressinfo_handle::Iterator::reference
addressinfo_handle::Iterator::operator*() const
{
  return *addrinfo_ptr;
}

addressinfo_handle::Iterator::pointer
addressinfo_handle::Iterator::operator->()
{
  return addrinfo_ptr;
}

/* prefix increment */
addressinfo_handle::Iterator&
addressinfo_handle::Iterator::operator++()
{
  addrinfo_ptr = addrinfo_ptr->ai_next;
  return *this;
}

bool
operator==(const addressinfo_handle::Iterator& lhs,
           const addressinfo_handle::Iterator& rhs)
{
  return lhs.addrinfo_ptr == rhs.addrinfo_ptr;
}

bool
operator!=(const addressinfo_handle::Iterator& lhs,
           const addressinfo_handle::Iterator& rhs)
{
  return !(lhs == rhs); // use the previously defined equality
}

addressinfo_handle::Iterator
addressinfo_handle::begin()
{
  return Iterator(info);
}

addressinfo_handle::Iterator
addressinfo_handle::end()
{
  return Iterator(end_p);
}

/* postfix increment */
addressinfo_handle::Iterator
addressinfo_handle::Iterator::operator++(int)
{
  Iterator old = *this;
  ++(*this); // call prefix increment
  return old;
}

/* struct managed_socket */

managed_socket::managed_socket()
  : socket_handle()
  , addressinfolist()
{
}

managed_socket::managed_socket(icysock::icy_socket s)
  : socket_handle(s)
  , addressinfolist()
{
}

managed_socket::managed_socket(managed_socket&& ms)
  : socket_handle(ms.socket_handle)
  , addressinfolist(ms.addressinfolist)
{
  ms.addressinfolist = nullptr;
}

managed_socket::managed_socket(const string_view hostname,
                               const string_view service,
                               const socket_hint hint)
  : addressinfolist(hostname, service, hint)
{
  for (auto ainfo : addressinfolist) {
    socket_handle =
      socket(ainfo.ai_family, ainfo.ai_socktype, ainfo.ai_protocol);

    if (socket_handle == BAD_SOCKET) {
      // socket is bad, this addrinfo structure didn't work
      // better try the next one until it works... or all of them fail.
      continue;
    }
    valid_addr = ainfo;
    break;
  }

  if (socket_handle == BAD_SOCKET) {
    /* we know the entire list is most likely empty */
    throw icysock_errors::SocketInitError(
      icysock_errors::errc::bad_addrinfolist,
      std::string("socket() failed due to") +
        std::string(std::strerror(errno)));
  }
}

void
managed_socket::bind_socket(bool reuse_socket)
{
  if (reuse_socket) {
    int enable = 1;
    if (setsockopt(
          socket_handle, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) ==
        SOCK_ERR) {
      throw icysock_errors::APIError(icysock_errors::errc::setsockopt_failure,
                                     std::string(std::strerror(errno)));
    }
  }

  if (bind(socket_handle, valid_addr.ai_addr, valid_addr.ai_addrlen) ==
      SOCK_ERR) {
    throw icysock_errors::APIError(icysock_errors::errc::bind_failure,
                                   std::string(std::strerror(errno)));
  }
}

} // namespace BetterSockets
