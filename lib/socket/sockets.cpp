#include "socket/sockets.hpp"
#include "socket/error_utils.hpp"

#include <cstdio>

using namespace std;
using std::fprintf;

namespace BetterSockets {

/*** Socket abstraction implementation ***/

/* struct socket_hint */
socket_hint::socket_hint(const ip_t i, const sock_t s, const flags_t f)
  : hostip_type(i)
  , socket_type(s)
  , flags(f)
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
  hints.ai_family = hint.hostip_type;
  hints.ai_socktype = hint.socket_type;
  hints.ai_flags = hint.flags;

  int rv = getaddrinfo(hostname.data(), service.data(), &hints, &info);
  if (rv != 0) {
    fprintf(stderr, "sockets: getaddrinfo(): %s\n", gai_strerror(rv));
    throw icysock_errors::SocketInitError(
      icysock_errors::errc::getaddrinfo_failure, gai_strerror(rv));
  }

  for (auto p = info; p->ai_next != nullptr; p = p->ai_next)
    end_p = p;
}

addressinfo_handle::~addressinfo_handle()
{
  freeaddrinfo(info);
  info = nullptr;
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
  , addressinfo()
{
}

managed_socket::managed_socket(const string_view hostname,
                               const string_view service,
                               const socket_hint hint)
  : addressinfo(hostname, service, hint)
{
  for (auto ainfo : addressinfo) {
    socket_handle =
      socket(ainfo.ai_family, ainfo.ai_socktype, ainfo.ai_protocol);

    if (socket_handle == BAD_SOCKET) {
      perror("managed_socket");
      continue;
    } // if
  }   // for

  /* we know the list is most likely empty */
  if (socket_handle == BAD_SOCKET) {
    throw icysock_errors::SocketInitError(icysock_errors::errc::empty_addrinfo);
  }
}

}