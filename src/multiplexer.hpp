#ifndef AVANTEE_MULTIPLEXER_H
#define AVANTEE_MULTIPLEXER_H

#include <array>
#include <utility>

#include "socket/generic_sockets.hpp"

#define TYPEOF(Value) __decltype(Value)

struct multiplexer
{

  enum class constants
  {
    MAX_SERVER_CONNECTIONS = 64,
    POLL_FOR = 0,
  };

  BetterSocket::size fdcount;
  std::array<BetterSocket::gpollfd,
             std::to_underlying(constants::MAX_SERVER_CONNECTIONS)>
    poll_over;

  enum class events : TYPEOF(TYPEOF(poll_over)::value_type::events){
    INPUT = POLLIN,
    OUTPUT = POLLOUT,
    ERROR = POLLERR,
    INVALID = POLLNVAL,
  };

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


template<multiplexer::events Event>
bool
multiplexer::socket_available_for(BetterSocket::gsocket sock)
{
  for (BetterSocket::size i = 0; i < fdcount; i++) {
    if ((poll_over[i].fd == sock) &&
        (poll_over[i].revents & std::to_underlying(Event)))
      return true;
  }
  return false;
}


#undef TYPEOF // i dont need you anymore :(

#endif
