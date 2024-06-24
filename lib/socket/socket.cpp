#include <algorithm>
#include <cerrno>
#include <cstring>
#include <sys/socket.h>

#include "socket/error_utils.hpp"
#include "socket/generic_sockets.hpp"
#include "socket/socket.hpp"

using namespace std;

namespace BetterSocket {

/*** Socket abstraction implementation ***/

/* struct SocketHint */
SocketHint::SocketHint(const IpVersion v,
                       const SockKind k,
                       const SockFlags f,
                       const IpProtocol ipr)
  : hostIpVersion(v)
  , socket_kind(k)
  , flags(f)
  , ipproto(ipr)
{
}

/* struct adressinfo_handle */

AddressinfoHandle::AddressinfoHandle()
  : beginP(nullptr)
  , endP(nullptr)
  , infoP(nullptr)
{
}

AddressinfoHandle::AddressinfoHandle(const string& hostname,
                                     const string& service,
                                     const SocketHint hint)
{
  struct addrinfo hints;
  BetterSocket::zero(&hints, sizeof(hints));
  hints.ai_family = static_cast<int>(hint.hostIpVersion);
  hints.ai_socktype = static_cast<int>(hint.socket_kind);
  if (hint.flags != SockFlags::None)
    hints.ai_flags = static_cast<int>(hint.flags);
  hints.ai_protocol = static_cast<int>(hint.ipproto);

  int rv = getaddrinfo(
    hostname.empty() ? NULL : hostname.data(), service.data(), &hints, &infoP);
  if (rv != 0) {
    throw SockErrors::SocketInitError(SockErrors::errc::getaddrinfo_failure,
                                      gai_strerror(rv));
  }

  /* store the pointer to the first last element of list */
  beginP = infoP;
  auto p = infoP;
  for (; p->ai_next != nullptr; p = p->ai_next)
    ;
  endP = p;
}

AddressinfoHandle::AddressinfoHandle(const AddressinfoHandle& h)
  : beginP{ h.beginP }
  , endP{ h.endP }
  , infoP{ h.infoP }
{
}

AddressinfoHandle::~AddressinfoHandle()
{
  freeaddrinfo(infoP);
  infoP = nullptr;
}

constexpr AddressinfoHandle&
AddressinfoHandle::operator=(const AddressinfoHandle& h)
{
  this->beginP = h.beginP;
  this->endP = h.endP;
  this->infoP = h.infoP;

  return *this;
}

/* AddressinfoHandle Iterator implementation */

AddressinfoHandle::Iterator::Iterator(AddressinfoHandle::Iterator::pointer iptr)
  : addrinfoPtr(iptr)
{
}

AddressinfoHandle::Iterator::reference
AddressinfoHandle::Iterator::operator*() const
{
  return *addrinfoPtr;
}

AddressinfoHandle::Iterator::pointer
AddressinfoHandle::Iterator::operator->()
{
  return addrinfoPtr;
}

/* prefix increment */
AddressinfoHandle::Iterator&
AddressinfoHandle::Iterator::operator++()
{
  addrinfoPtr = addrinfoPtr->ai_next;
  return *this;
}

/* postfix increment */
AddressinfoHandle::Iterator
AddressinfoHandle::Iterator::operator++(int)
{
  Iterator old = *this;
  ++(*this); // call prefix increment
  return old;
}

bool
operator==(const AddressinfoHandle::Iterator& lhs,
           const AddressinfoHandle::Iterator& rhs)
{
  return lhs.addrinfoPtr == rhs.addrinfoPtr;
}

bool
operator!=(const AddressinfoHandle::Iterator& lhs,
           const AddressinfoHandle::Iterator& rhs)
{
  return !(lhs == rhs); // use the previously defined equality
}

AddressinfoHandle::Iterator
AddressinfoHandle::begin()
{
  return Iterator(beginP);
}

AddressinfoHandle::Iterator
AddressinfoHandle::end()
{
  return Iterator(endP + 1);
}

bool
operator==(const AddressinfoHandle& lhs, const AddressinfoHandle& rhs)
{
  return lhs.beginP == rhs.beginP && lhs.endP == rhs.endP;
}

bool
operator!=(const AddressinfoHandle& lhs, const AddressinfoHandle& rhs)
{
  return !(lhs == rhs);
}

void
AddressinfoHandle::next()
{
  if (infoP->ai_next == nullptr)
    throw SockErrors::APIError(SockErrors::errc::bad_addrinfolist,
                               "Reached the end of list.");

  infoP = infoP->ai_next;
}

// finish AddressinfoHandle

// struct SockaddrWrapper Implementation

SockaddrWrapper::SockaddrWrapper()
  : wrappingOverIP(IpVersion::vAny)
  , sockaddrsz(0)
  , genericSockaddr()
  , ipv4Sockaddr()
  , ipv6Sockaddr()
  , IsSetIPCalled(false)
{
}

SockaddrWrapper::SockaddrWrapper(sockaddr s, socklen_t size)
  : sockaddrsz(size)
  , genericSockaddr(*(reinterpret_cast<sockaddr_storage*>(&s)))
{
  m_setIP();
}

SockaddrWrapper::SockaddrWrapper(sockaddr_in s4, socklen_t size)
  : wrappingOverIP(IpVersion::v4)
  , sockaddrsz(size)
  , ipv4Sockaddr(s4)
  , IsSetIPCalled(true)
{
}

SockaddrWrapper::SockaddrWrapper(sockaddr_in6 s6, socklen_t size)
  : wrappingOverIP(IpVersion::v6)
  , sockaddrsz(size)
  , ipv6Sockaddr(s6)
  , IsSetIPCalled(true)
{
}

SockaddrWrapper::SockaddrWrapper(sockaddr_in& s4, socklen_t size)
  : wrappingOverIP(IpVersion::v4)
  , sockaddrsz(size)
  , ipv4Sockaddr(s4)
  , IsSetIPCalled(true)
{
}

SockaddrWrapper::SockaddrWrapper(sockaddr_in6& s6, socklen_t size)
  : wrappingOverIP(IpVersion::v6)
  , sockaddrsz(size)
  , ipv6Sockaddr(s6)
  , IsSetIPCalled(true)
{
}

SockaddrWrapper::SockaddrWrapper(sockaddr_storage& gs, socklen_t size)
  : sockaddrsz(size)
  , genericSockaddr(gs)
{
  m_setIP();
}

const struct sockaddr_in*
SockaddrWrapper::getPtrToV4()
{
  return &ipv4Sockaddr;
}

const struct sockaddr_in6*
SockaddrWrapper::getPtrToV6()
{
  return &ipv6Sockaddr;
}

in_port_t
SockaddrWrapper::getPort() const
{
  if (IsSetIPCalled) {
    if (wrappingOverIP == IpVersion::v4)
      return ipv4Sockaddr.sin_port;
    return ipv6Sockaddr.sin6_port;
  }
  throw SockErrors::APIError(
    SockErrors::errc::ipfamily_not_set,
    std::string(
      "Probably improperly using this wrapper since the m_setIP function is "
      "supposed to be called by BSocket::receiveFrom() or BSocket::sendTo()."));
}

std::string
SockaddrWrapper::getIP() const
{
  if (IsSetIPCalled) {
    auto zeroToSize = [](std::string& buf, unsigned int size) {
      buf.resize(size);
      std::fill(buf.begin(), buf.end(), 0);
      return size;
    };

    std::string ipBuffer;
    if (wrappingOverIP == IpVersion::v4) {
      auto rsize = zeroToSize(ipBuffer, INET_ADDRSTRLEN);
      inet_ntop(ipv4Sockaddr.sin_family,
                &ipv4Sockaddr.sin_addr,
                ipBuffer.data(),
                rsize);
      return ipBuffer;
    }

    auto rsize = zeroToSize(ipBuffer, INET6_ADDRSTRLEN);
    inet_ntop(ipv6Sockaddr.sin6_family,
              &ipv6Sockaddr.sin6_addr,
              ipBuffer.data(),
              rsize);
    return ipBuffer;
  }

  throw SockErrors::APIError(
    SockErrors::errc::ipfamily_not_set,
    std::string(
      "Probably improperly using this wrapper since the m_setIP function is "
      "supposed to be called by BSocket::receiveFrom() or BSocket::sendTo()."));
}

void
SockaddrWrapper::m_setIP()
{
  wrappingOverIP = static_cast<IpVersion>(genericSockaddr.ss_family);
  if ((wrappingOverIP != IpVersion::v4) and (wrappingOverIP != IpVersion::v6))
    throw SockErrors::APIError(SockErrors::errc::ipfamily_not_set,
                               strerror(errno));
  if (wrappingOverIP == IpVersion::v4)
    this->ipv4Sockaddr = *reinterpret_cast<sockaddr_in*>(
      &genericSockaddr); // kekw wtf is this garbage c++
  this->ipv6Sockaddr = *reinterpret_cast<sockaddr_in6*>(&genericSockaddr);
  IsSetIPCalled = true;
}

sockaddr_storage*
SockaddrWrapper::m_getPtrToStorage()
{
  return &this->genericSockaddr;
}

// finish SockaddrWrapper

/******** struct BSocket ************/

BSocket::BSocket()
  : bindsCalled(false)
  , empty(false)
  , IsListener(false)
  , addressinfoList()
  , validAddr()
  , rawSocket(BAD_SOCKET)
{
}

BSocket::BSocket(BetterSocket::GSocket s)
  : bindsCalled(false)
  , empty(false)
  , IsListener(false)
  , addressinfoList()
  , validAddr()
  , rawSocket(s)
{
  if (s == SOCK_ERR)
    throw SockErrors::SocketInitError(
      SockErrors::errc::bad_socket,
      std::string("Creation of BSocket failed. Invalid argument."));
}

BSocket::BSocket(BSocket&& ms)
  : bindsCalled(ms.bindsCalled)
  , empty(ms.empty)
  , IsListener(ms.IsListener)
  , addressinfoList(ms.addressinfoList)
  , validAddr()
  , rawSocket(ms.rawSocket)
{
}

BSocket::BSocket(const struct SocketHint hint,
                 const string& service,
                 const string& hostname)
  : bindsCalled(false)
  , empty(false)
  , IsListener(false)
  , addressinfoList(hostname, service, hint)
  , validAddr()
  , rawSocket(BAD_SOCKET)
{
  /* This commented out snippet of code needs to be fixed.
   * I think that the iterators for addresinfoP_handle structure aare broken.
   * But I shall take another look at it after I'm done testing out some code.
   * Then this shall be fixed.
   */
  for (auto& ainfoP : addressinfoList) {
    initRawSocket(&ainfoP);
    // socket is bad, this addrinfo structure didn't work
    // better try the next one until it works... or all of them fail.
    if (rawSocket == BAD_SOCKET)
      continue;

    validAddr = ainfoP;
    break;
  }

  if (rawSocket == BAD_SOCKET) {
    /* we know the entire list is most likely empty */
    throw SockErrors::SocketInitError(SockErrors::errc::bad_addrinfolist,
                                      std::string("socket() failed due to") +
                                        std::string(std::strerror(errno)));
  }
}

BSocket::~BSocket()
{
  BetterSocket::closeSocket(rawSocket);
  empty = true;
  IsListener = false;
  bindsCalled = false;
  rawSocket = BAD_SOCKET;
}

void
BSocket::initRawSocket(struct addrinfo* addr)
{
  rawSocket = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
}

bool
BSocket::IsEmpty() const
{
  return empty;
}

GSocket
BSocket::underlyingSocket() const
{
  return rawSocket;
}

void
BSocket::clearOut()
{
  *this = BSocket();
}

BSocket&
BSocket::operator=(BSocket&& s)
{
  bindsCalled = s.bindsCalled;
  empty = s.empty;
  IsListener = s.IsListener;
  addressinfoList = s.addressinfoList;
  validAddr = s.validAddr;
  rawSocket = s.rawSocket;
  // s.clearOut();
  return *this;
}

bool
operator==(const int& lhs, const BSocket& rhs)
{
  return lhs == rhs.underlyingSocket();
}

bool
operator!=(const int& lhs, const BSocket& rhs)
{
  return !(lhs == rhs);
}

bool
operator==(const BSocket& lhs, const int& rhs)
{
  return lhs.underlyingSocket() == rhs;
}

bool
operator!=(const BSocket& lhs, const int& rhs)
{
  return !(lhs == rhs);
}

bool
operator==(const BSocket& lhs, const BSocket& rhs)
{
  return lhs.empty == rhs.empty && lhs.addressinfoList == rhs.addressinfoList &&
         lhs.underlyingSocket() == rhs.underlyingSocket();
}

bool
operator!=(const BSocket& lhs, const BSocket& rhs)
{
  return !(lhs == rhs); // use the previously defined equality
}

sockaddr
BSocket::getsockaddr() const
{
  return *validAddr.ai_addr;
}

SockaddrWrapper
BSocket::getsockaddrInWrapper() const
{
  return SockaddrWrapper(*validAddr.ai_addr);
}

void
BSocket::tryNext()
{
  BetterSocket::closeSocket(rawSocket);

  try {
    addressinfoList.next();
  } catch (const SockErrors::APIError& e) {
    throw;
  }

  validAddr = *addressinfoList.infoP;
  initRawSocket(addressinfoList.infoP);
  if (rawSocket == BAD_SOCKET) {
    throw SockErrors::APIError(
      SockErrors::errc::bad_socket,
      "initialising socket failed while trying the next 'struct addrinfo'");
  }
  bindsCalled = false;
  empty = false;
  IsListener = false;
}

/* -- socket api -- */
/* implement arg2 and arg3 at a later date, more infoP in header file. */
BetterSocket::GSocket BSocket::acceptS(/*, arg2, arg3 */)
{
  /* Only call accept() on the socket if listen() has been called before.
   * Otherwise do nothing and return an invalid socket. */
  if (IsListener) {
    BetterSocket::GSocket accepted = accept(rawSocket, nullptr, nullptr);
    if (accepted == SOCK_ERR)
      throw SockErrors::APIError(SockErrors::errc::accept_failure,
                                 std::string(std::strerror(errno)));

    return accepted;
  }

  return SOCK_ERR;
}

void
BSocket::bindS(bool reuseSocket)
{
  if (reuseSocket) {
    int enable = 1;
    if (setsockopt(rawSocket,
                   SOL_SOCKET,
                   SO_REUSEADDR,
#ifdef ICY_ON_WINDOWS
                   reinterpret_cast<char*>(&enable),
#else
                   &enable,
#endif

                   sizeof(enable)) == SOCK_ERR)
      throw SockErrors::APIError(SockErrors::errc::setsockopt_failure,
                                 std::string(std::strerror(errno)));
  }

  if (bind(rawSocket, validAddr.ai_addr, validAddr.ai_addrlen) == SOCK_ERR)
    throw SockErrors::APIError(SockErrors::errc::bind_failure,
                               std::string(std::strerror(errno)));

  bindsCalled = true;
}

void
BSocket::connectS()
{
  if (connect(rawSocket, validAddr.ai_addr, validAddr.ai_addrlen) == SOCK_ERR) {
    throw SockErrors::APIError(SockErrors::errc::connect_failure,
                               std::string(std::strerror(errno)));
  }
}

void
BSocket::listenS()
{
  /* call bindS if it has not been called before */
  if (!bindsCalled)
    bindS();
  if (listen(rawSocket, SOMAXCONN) == SOCK_ERR)
    throw SockErrors::APIError(SockErrors::errc::listen_failure,
                               std::string(std::strerror(errno)));

  IsListener = true;
}

BetterSocket::SSize
BSocket::receive(void* buf, BetterSocket::Size s, int flags)
{
  BetterSocket::SSize r = recv(rawSocket,
#ifdef ICY_ON_WINDOWS
                               reinterpret_cast<char*>(buf),
#else
                               buf,
#endif

                               (size_t)s,
                               flags);
  if (r == SOCK_ERR)
    throw SockErrors::APIError(SockErrors::errc::receive_failure,
                               std::string(std::strerror(errno)));

  return r;
}

BetterSocket::SSize
BSocket::receiveFrom(void* ibuf,
                     BetterSocket::Size bufsz,
                     SockaddrWrapper& senderAddr,
                     int flags)
{
  socklen_t senderSz = sizeof(*senderAddr.m_getPtrToStorage());
  BetterSocket::SSize s =
    recvfrom(rawSocket,
#ifdef ICY_ON_WINDOWS
             reinterpret_cast<char*>(ibuf),
#else
             ibuf,
#endif
             bufsz,
             flags,
             reinterpret_cast<sockaddr*>(senderAddr.m_getPtrToStorage()),
#ifdef ICY_ON_WINDOWS
             reinterpret_cast<int*>(&senderSz)
#else
             &senderSz
#endif
    );
  if (s == SOCK_ERR)
    throw SockErrors::APIError(SockErrors::errc::receive_from_failure,
                               std::string(std::strerror(errno)));

  senderAddr.sockaddrsz = senderSz;
  senderAddr.m_setIP();

  return s;
}

BetterSocket::SSize
BSocket::sendS(std::string_view ibuf, int flags)
{
  BetterSocket::SSize r = send(rawSocket, ibuf.data(), ibuf.length(), flags);
  if (r == SOCK_ERR)
    throw SockErrors::APIError(SockErrors::errc::send_failure,
                               std::string(std::strerror(errno)));

  return r;
}

BetterSocket::SSize
BSocket::sendTo(void* ibuf,
                BetterSocket::Size bufsz,
                SockaddrWrapper& destAddr,
                int flags)
{
  unsigned int destSz = destAddr.sockaddrsz;
  BetterSocket::SSize r =
    sendto(this->rawSocket,
#ifdef ICY_ON_WINDOWS
           reinterpret_cast<char*>(ibuf),
#else
           ibuf,
#endif
           bufsz,
           flags,
           reinterpret_cast<sockaddr*>(destAddr.m_getPtrToStorage()),
           destSz);
  if (r == SOCK_ERR)
    throw SockErrors::APIError(SockErrors::errc::sendto_failure,
                               std::string(std::strerror(errno)));
  destAddr.m_setIP();

  return r;
}

void
BSocket::shutdownS(enum BetterSocket::TransmissionEnd reason)
{
  if (shutdown(rawSocket, static_cast<int>(reason)) == SOCK_ERR)
    throw SockErrors::APIError(SockErrors::errc::shutdown_failure,
                               std::string(std::strerror(errno)));
}
// finish BSocket
} // namespace BetterSocket
