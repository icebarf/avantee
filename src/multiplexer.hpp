#ifndef AVANTEE_MULTIPLEXER_H
#define AVANTEE_MULTIPLEXER_H

#include <array>
#include <poll.h>

#include "socket/generic_sockets.hpp"

struct multiplexer
{
  enum
  {
    MAX_SERVER_CONNECTIONS = 64,
    POLL_FOR = 0,
  };

  enum events
  {
    INPUT = POLLIN,
    OUTPUT = POLLOUT,
    ERROR = POLLERR,
    INVALID = POLLNVAL,
  };
  BetterSocket::size fdcount;
  std::array<BetterSocket::gpollfd, MAX_SERVER_CONNECTIONS> poll_over;
  std::array<BetterSocket::gsocket, MAX_SERVER_CONNECTIONS> readable;
  std::array<BetterSocket::gsocket, MAX_SERVER_CONNECTIONS> writable;

  multiplexer();
  
  /* add socket to be polled over */
  void watch(BetterSocket::gsocket socket, events ev);
  /* remove socket from polling */
  void unwatch(BetterSocket::gsocket socket);
  /* update the event for socket to be polled over */
  void update_fd_event(BetterSocket::gsocket socket, events ev);

  /* poll for I/O on `poll_over` */
  void poll_io();

  /* check if a socket is available for some event or not */
  template<multiplexer::events Event>
  bool socket_available_for(BetterSocket::gsocket sock);
};

#endif
