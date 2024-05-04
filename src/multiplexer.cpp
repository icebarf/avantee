#include "multiplexer.hpp"
#include "socket/generic_sockets.hpp"

#include <cstdio>
#include <exception>

multiplexer::multiplexer()
  : poll_over{}
{
  poll_over.fill(BetterSocket::gpollfd(-1, 0, 0));
}

void
multiplexer::watch(BetterSocket::gsocket socket, events ev)
{
  poll_over[fdcount].fd = socket;
  poll_over[fdcount++].events = ev;
}

void
multiplexer::unwatch(BetterSocket::gsocket socket)
{
  for (BetterSocket::size i = 0; i < fdcount; i++) {
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
multiplexer::update_fd_event(BetterSocket::gsocket socket, events ev)
{
  for (BetterSocket::size i = 0; i < fdcount; i++) {
    if (poll_over[i].fd == socket)
      poll_over[i].events = ev;
  }
}

void
multiplexer::poll_io()
{
  int polled =
    BetterSocket::gpoll(poll_over.data(), fdcount, multiplexer::POLL_FOR);

  if (polled == SOCK_ERR) {
    std::perror("mutiplexer::poll_io -> poll()");
    std::terminate();
  }
}

template<multiplexer::events Event>
bool
multiplexer::socket_available_for(BetterSocket::gsocket sock)
{
  for (BetterSocket::size i = 0; i < fdcount; i++) {
    if (poll_over[i].fd == sock && poll_over[i].revents && Event)
      return true;
  }
  return false;
}
