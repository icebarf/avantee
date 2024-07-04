#include <cstdio>
#include <exception>
#include <utility>

#include "multiplexer.hpp"
#include "socket/generic_sockets.hpp"

#define SCAST(Type, e) static_cast<Type>(e)
#define TU(e) std::to_underlying(e)

Multiplexer::Multiplexer()
  : fdcount{ 0 }
  , poll_over{}
{
  poll_over.fill(BetterSocket::GPollfd(-1, 0, 0));
}

void
Multiplexer::watch(BetterSocket::GSocket socket, Events ev)
{
  poll_over[fdcount].fd = socket;
  poll_over[fdcount++].events = std::to_underlying(ev);
}

void
Multiplexer::unwatch(BetterSocket::GSocket socket)
{
  for (BetterSocket::Size i = 0; i < fdcount; i++) {
    if (poll_over[i].fd == socket) {
      // if only watching over one socket, then ignore.
      // otherwise copy over it
      if (i == 0)
        poll_over[i].fd = -1; // poll ignores -1 fds

      poll_over[i] = poll_over[fdcount - 1];
      fdcount--;
    }
  }
}

void
Multiplexer::update_fd_event(BetterSocket::GSocket socket, Events ev)
{
  for (BetterSocket::Size i = 0; i < fdcount; i++) {
    if (poll_over[i].fd == socket)
      poll_over[i].events = std::to_underlying(ev);
  }
}

void
Multiplexer::poll_io()
{
  int polled = BetterSocket::gPoll(
    poll_over.data(), fdcount, SCAST(int, constants::POLL_FOR));

  if (polled == SOCK_ERR) {
    std::perror("mutiplexer::poll_io -> poll()");
    std::terminate();
  }
}

// overload definition
unsigned long
operator*(const Multiplexer::constants& c, unsigned long v)
{
  return TU(c) * v;
}
