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
  : begin_p(nullptr)
  , end_p(nullptr)
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
  if (hint.flags != sock_flags::NONE)
    hints.ai_flags = static_cast<int>(hint.flags);
  hints.ai_protocol = static_cast<int>(hint.ipproto);

  int rv = getaddrinfo(
    hostname.empty() ? NULL : hostname.data(), service.data(), &hints, &info);
  if (rv != 0) {
    throw sock_errors::SocketInitError(icysock::errors::getaddrinfo_failure,
                                       gai_strerror(rv));
  }

  /* store the pointer to the first last element of list */
  begin_p = info;
  auto p = info;
  for (; p->ai_next != nullptr; p = p->ai_next)
    ;
  end_p = p;
}

addressinfo_handle::addressinfo_handle(const addressinfo_handle& h)
  : begin_p{ h.begin_p }
  , end_p{ h.end_p }
  , info{ h.info }
{
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
  return Iterator(begin_p);
}

addressinfo_handle::Iterator
addressinfo_handle::end()
{
  return Iterator(end_p + 1);
}

bool
operator==(const addressinfo_handle& lhs, const addressinfo_handle& rhs)
{
  return lhs.begin_p == rhs.begin_p && lhs.end_p == rhs.end_p;
}

bool
operator!=(const addressinfo_handle& lhs, const addressinfo_handle& rhs)
{
  return !(lhs == rhs);
}

void
addressinfo_handle::next()
{
  if (info->ai_next == nullptr)
    throw sock_errors::APIError(sock_errors::errc::bad_addrinfolist,
                                "Reached the end of list.");

  info = info->ai_next;
}

// finish addressinfo_handle

/******** struct managed_socket ************/

managed_socket::managed_socket()
  : binds_called(false)
  , empty(false)
  , is_listener(false)
  , addressinfolist()
  , socket_handle(BAD_SOCKET)
{
}

managed_socket::managed_socket(icysock::gsocket s)
  : binds_called(false)
  , empty(false)
  , is_listener(false)
  , addressinfolist()
  , socket_handle(s)
{
  if (s == SOCK_ERR)
    throw sock_errors::SocketInitError(
      sock_errors::errc::bad_socket,
      std::string("Creation of managed_socket failed. Invalid argument."));
}

managed_socket::managed_socket(managed_socket&& ms)
  : binds_called(ms.binds_called)
  , empty(ms.empty)
  , is_listener(ms.is_listener)
  , addressinfolist(ms.addressinfolist)
  , socket_handle(ms.socket_handle)
{
}

managed_socket::managed_socket(const struct socket_hint hint,
                               const string service,
                               const string hostname)
  : binds_called(false)
  , empty(false)
  , is_listener(false)
  , addressinfolist(hostname, service, hint)
  , socket_handle{ BAD_SOCKET }
{
  /* This commented out snippet of code needs to be fixed.
   * I think that the iterators for addresinfo_handle structure aare broken.
   * But I shall take another look at it after I'm done testing out some code.
   * Then this shall be fixed.
   */
  for (auto& ainfo : addressinfolist) {
    init_socket_handle(&ainfo);
    // socket is bad, this addrinfo structure didn't work
    // better try the next one until it works... or all of them fail.
    if (socket_handle == BAD_SOCKET)
      continue;

    valid_addr = ainfo;
    break;
  }

  if (socket_handle == BAD_SOCKET) {
    /* we know the entire list is most likely empty */
    throw sock_errors::SocketInitError(sock_errors::errc::bad_addrinfolist,
                                       std::string("socket() failed due to") +
                                         std::string(std::strerror(errno)));
  }
}

managed_socket::~managed_socket()
{
  icysock::close_socket(socket_handle);
  empty = true;
  is_listener = false;
  binds_called = false;
}

void
managed_socket::init_socket_handle(struct addrinfo* addr)
{
  socket_handle = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
}

bool
managed_socket::is_empty()
{
  return empty;
}

bool
operator==(const int& lhs, const managed_socket& rhs)
{
  return lhs == rhs.socket_handle;
}

bool
operator!=(const int& lhs, const managed_socket& rhs)
{
  return !(lhs == rhs);
}

bool
operator==(const managed_socket& lhs, const int& rhs)
{
  return lhs.socket_handle == rhs;
}

bool
operator!=(const managed_socket& lhs, const int& rhs)
{
  return !(lhs == rhs);
}

bool
operator==(const managed_socket& lhs, const managed_socket& rhs)
{
  return lhs.empty == rhs.empty && lhs.addressinfolist == rhs.addressinfolist &&
         lhs.socket_handle == rhs.socket_handle;
}

bool
operator!=(const managed_socket& lhs, const managed_socket& rhs)
{
  return !(lhs == rhs); // use the previously defined equality
}

/* implement arg2 and arg3 at a later date, more info in header file. */
icysock::gsocket managed_socket::accepts(/*, arg2, arg3 */)
{
  /* Only call accept() on the socket if listen() has been called before.
   * Otherwise do nothing and return an invalid socket. */
  if (is_listener) {
    icysock::gsocket accepted = accept(socket_handle, nullptr, nullptr);
    if (accepted == SOCK_ERR) {
      throw sock_errors::APIError(sock_errors::errc::accept_failure,
                                  std::string(std::strerror(errno)));
    }
    return accepted;
  }

  return SOCK_ERR;
}

void
managed_socket::binds(bool reuse_socket)
{
  if (reuse_socket) {
    int enable = 1;
    if (setsockopt(
          socket_handle, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) ==
        SOCK_ERR) {
      throw sock_errors::APIError(sock_errors::errc::setsockopt_failure,
                                  std::string(std::strerror(errno)));
    }
  }

  if (bind(socket_handle, valid_addr.ai_addr, valid_addr.ai_addrlen) ==
      SOCK_ERR) {
    throw sock_errors::APIError(sock_errors::errc::bind_failure,
                                std::string(std::strerror(errno)));
  }

  binds_called = true;
}

void
managed_socket::connects()
{
  if (connect(socket_handle, valid_addr.ai_addr, valid_addr.ai_addrlen) ==
      SOCK_ERR) {
    throw sock_errors::APIError(sock_errors::errc::connect_failure,
                                std::string(std::strerror(errno)));
  }
}

void
managed_socket::listens()
{
  /* call binds if it has not been called before */
  if (!binds_called)
    binds();
  if (listen(socket_handle, SOMAXCONN) == SOCK_ERR) {
    throw sock_errors::APIError(sock_errors::errc::listen_failure,
                                std::string(std::strerror(errno)));
  }
  is_listener = true;
}

icysock::ssize
managed_socket::receive(void* buf, icysock::size s, int flags)
{
  icysock::ssize r = recv(socket_handle, buf, (size_t)s, flags);
  if (r == SOCK_ERR) {
    throw sock_errors::APIError(sock_errors::errc::receive_failure,
                                std::string(std::strerror(errno)));
  }
  return r;
}

icysock::ssize
managed_socket::receive_from(void* ibuf,
                             icysock::size bufsz,
                             struct sockaddr* sender_addr,
                             icysock::size* sndrsz,
                             int flags)
{
  icysock::ssize s = recvfrom(socket_handle,
                              ibuf,
                              bufsz,
                              flags,
                              sender_addr,
                              reinterpret_cast<unsigned int*>(sndrsz));
  if (s == SOCK_ERR) {
    throw sock_errors::APIError(sock_errors::errc::receive_from_failure,
                                std::string(std::strerror(errno)));
  }

  return s;
}

icysock::ssize
managed_socket::sends(std::string_view ibuf, int flags)
{
  icysock::ssize r = send(socket_handle, ibuf.data(), ibuf.length(), flags);
  if (r == SOCK_ERR) {
    throw sock_errors::APIError(sock_errors::errc::send_failure,
                                std::string(std::strerror(errno)));
  }

  return r;
}

icysock::ssize
managed_socket::send_to(void* ibuf,
                        icysock::size bufsz,
                        struct sockaddr* dest_addr,
                        icysock::size destsz,
                        int flags)
{
  icysock::ssize r =
    sendto(this->socket_handle, ibuf, bufsz, flags, dest_addr, destsz);
  if (r == SOCK_ERR) {
    throw sock_errors::APIError(sock_errors::errc::sendto_failure,
                                std::string(std::strerror(errno)));
  }
  return r;
}

void
managed_socket::shutdowns(enum BetterSockets::TransmissionEnd reason)
{
  if (shutdown(socket_handle, static_cast<int>(reason)) == SOCK_ERR) {
    throw sock_errors::APIError(sock_errors::errc::shutdown_failure,
                                std::string(std::strerror(errno)));
  }
}

void
managed_socket::try_next()
{
  icysock::close_socket(socket_handle);

  try {
    addressinfolist.next();
  } catch (const sock_errors::APIError& e) {
    throw;
  }

  valid_addr = *addressinfolist.info;
  init_socket_handle(addressinfolist.info);
  if (socket_handle == BAD_SOCKET) {
    throw sock_errors::APIError(
      sock_errors::errc::bad_socket,
      "initialising socket failed while trying the next 'struct addrinfo'");
  }
  binds_called = false;
  empty = false;
  is_listener = false;
}
// finish managed_socket

/* implementation of struct multiplexer*/
BetterSockets::multiplexer::multiplexer()
  : timeout{ 0, 0 }
  , watching_over{ 0 }
{
}

BetterSockets::multiplexer::multiplexer(long sec, long usec, int watch_over)
  : timeout(sec, usec)
  , watching_over(watch_over)
{
}

int&
BetterSockets::multiplexer::operator++()
{
  return ++watching_over;
}

int
BetterSockets::multiplexer::operator++(int)
{
  int old = watching_over;
  ++(*this); // prefix increment
  return old;
}

} // namespace BetterSockets
