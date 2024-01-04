#include <cerrno>
#include <cstring>
#include <sys/socket.h>

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

addressinfo_handle::addressinfo_handle(const string hostname,
                                       const string service,
                                       const socket_hint hint)
{
  struct addrinfo hints;
  icysock::zero(&hints, sizeof(hints));
  hints.ai_family = static_cast<int>(hint.hostip_version);
  hints.ai_socktype = static_cast<int>(hint.socket_kind);
  hints.ai_flags = static_cast<int>(hint.flags);
  hints.ai_protocol = static_cast<int>(hint.ipproto);

  int rv = getaddrinfo(
    hostname.empty() ? NULL : hostname.data(), service.data(), &hints, &info);
  if (rv != 0) {
    throw icysock::errors::SocketInitError(
      icysock::errors::errc::getaddrinfo_failure, gai_strerror(rv));
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

/* postfix increment */
addressinfo_handle::Iterator
addressinfo_handle::Iterator::operator++(int)
{
  Iterator old = *this;
  ++(*this); // call prefix increment
  return old;
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
  return Iterator(end_p + 1);
}

/* struct managed_socket */

managed_socket::managed_socket()
  : binds_called(false)
  , empty(false)
  , is_listener(false)
  , socket_handle(BAD_SOCKET)
  , addressinfolist()
{
}

managed_socket::managed_socket(icysock::gsocket s)
  : binds_called(false)
  , empty(false)
  , is_listener(false)
  , socket_handle(s)
  , addressinfolist()
{
  if (s == SOCK_ERR)
    throw icysock::errors::SocketInitError(
      icysock::errors::errc::bad_socket,
      std::string("Creation of managed_socket failed. Invalid argument."));
}

managed_socket::managed_socket(managed_socket&& ms)
  : binds_called(ms.binds_called)
  , empty(ms.empty)
  , is_listener(ms.is_listener)
  , socket_handle(ms.socket_handle)
  , addressinfolist(ms.addressinfolist)
{
  ms.addressinfolist = nullptr;
}

managed_socket::managed_socket(const struct socket_hint hint,
                               const string service,
                               const string hostname)
  : binds_called(false)
  , empty(false)
  , is_listener(false)
  , socket_handle{ BAD_SOCKET }
  , addressinfolist(hostname, service, hint)
{
  /* This commented out snippet of code needs to be fixed.
   * I think that the iterators for addresinfo_handle structure aare broken.
   * But I shall take another look at it after I'm done testing out some code.
   * Then this shall be fixed.
   */
  for (auto& ainfo : addressinfolist) {
    socket_handle =
      socket(ainfo.ai_family, ainfo.ai_socktype, ainfo.ai_protocol);

    // socket is bad, this addrinfo structure didn't work
    // better try the next one until it works... or all of them fail.
    if (socket_handle == BAD_SOCKET)
      continue;

    valid_addr = ainfo;
    break;
  }

  if (socket_handle == BAD_SOCKET) {
    /* we know the entire list is most likely empty */
    throw icysock::errors::SocketInitError(
      icysock::errors::errc::bad_addrinfolist,
      std::string("socket() failed due to") +
        std::string(std::strerror(errno)));
  }
}

managed_socket::~managed_socket()
{
  // if(is_listener) unlink(*name)
  this->shutdowns(BetterSockets::TransmissionEnd::EVERYTHING);
  icysock::close_socket(socket_handle);
  empty = true;
  is_listener = false;
  binds_called = false;
}

bool
managed_socket::is_empty()
{
  return empty;
}

void
managed_socket::binds(bool reuse_socket)
{
  if (reuse_socket) {
    int enable = 1;
    if (setsockopt(
          socket_handle, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) ==
        SOCK_ERR) {
      throw icysock::errors::APIError(icysock::errors::errc::setsockopt_failure,
                                      std::string(std::strerror(errno)));
    }
  }

  if (bind(socket_handle, valid_addr.ai_addr, valid_addr.ai_addrlen) ==
      SOCK_ERR) {
    throw icysock::errors::APIError(icysock::errors::errc::bind_failure,
                                    std::string(std::strerror(errno)));
  }

  binds_called = true;
}

void
managed_socket::connects()
{
  if (connect(socket_handle, valid_addr.ai_addr, valid_addr.ai_addrlen) ==
      SOCK_ERR) {
    throw icysock::errors::APIError(icysock::errors::errc::connect_failure,
                                    std::string(std::strerror(errno)));
  }
}

icysock::ssize
managed_socket::sends(std::string_view buf, int flags)
{
  icysock::ssize r = send(socket_handle, buf.data(), buf.length(), flags);
  if (r == SOCK_ERR) {
    throw icysock::errors::APIError(icysock::errors::errc::send_failure,
                                    std::string(std::strerror(errno)));
  }

  return r;
}

void
managed_socket::shutdowns(enum BetterSockets::TransmissionEnd reason)
{
  if (shutdown(socket_handle, static_cast<int>(reason)) == SOCK_ERR) {
    throw icysock::errors::APIError(icysock::errors::errc::shutdown_failure,
                                    std::string(std::strerror(errno)));
  }
}

icysock::ssize
managed_socket::receive(char* buf, icysock::ssize s, int flags)
{
  icysock::ssize r = recv(socket_handle, buf, (size_t)s, flags);
  if (r == SOCK_ERR) {
    throw icysock::errors::APIError(icysock::errors::errc::receieve_failure,
                                    std::string(std::strerror(errno)));
  }
  return r;
}

void
managed_socket::listens()
{
  /* call binds if it has not been called before */
  if (!binds_called)
    binds();
  if (listen(socket_handle, SOMAXCONN) == SOCK_ERR) {
    throw icysock::errors::APIError(icysock::errors::errc::listen_failure,
                                    std::string(std::strerror(errno)));
  }
  is_listener = true;
}

/* implement arg2 and arg3 at a later date, more info in header file. */
icysock::gsocket managed_socket::accepts(/*, arg2, arg3 */)
{
  /* Only call accept() on the socket if listen() has been called before.
   * Otherwise do nothing and return an invalid socket. */
  if (is_listener) {
    icysock::gsocket accepted = accept(socket_handle, nullptr, nullptr);
    if (accepted == SOCK_ERR) {
      throw icysock::errors::APIError(icysock::errors::errc::accept_failure,
                                      std::string(std::strerror(errno)));
    }
    return accepted;
  }

  return SOCK_ERR;
}

} // namespace BetterSockets
