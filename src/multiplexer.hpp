#ifndef AVANTEE_MULTIPLEXER_H
#define AVANTEE_MULTIPLEXER_H

#include <array>
#include <utility>

#include "socket/generic_sockets.hpp"

#define TYPEOF(Value) __decltype(Value)

struct Multiplexer
{

  enum class constants
  {
    MAX_SERVER_CONNECTIONS = 64,
    POLL_FOR = 0,
  };

  BetterSocket::Size fdcount;
  std::array<BetterSocket::GPollfd,
             std::to_underlying(constants::MAX_SERVER_CONNECTIONS)>
    poll_over;

  enum class Events : TYPEOF(TYPEOF(poll_over)::value_type::events){
    input = POLLIN,
    output = POLLOUT,
    error = POLLERR,
    invalid = POLLNVAL,
  };

  Multiplexer();

  /* add socket to be polled over */
  void watch(BetterSocket::GSocket socket, Events ev);
  /* remove socket from polling */
  void unwatch(BetterSocket::GSocket socket);
  /* update the event for socket to be polled over */
  void update_fd_event(BetterSocket::GSocket socket, Events ev);

  /* poll for I/O on `poll_over` */
  void poll_io();

  /* check if a socket is available for some event or not */
  template<Multiplexer::Events Event>
  bool socket_available_for(BetterSocket::GSocket sock);
};

template<Multiplexer::Events Event>
bool
Multiplexer::socket_available_for(BetterSocket::GSocket sock)
{
  for (BetterSocket::Size i = 0; i < fdcount; i++) {
    if ((poll_over[i].fd == sock) &&
        (poll_over[i].revents & std::to_underlying(Event)))
      return true;
  }
  return false;
}

#undef TYPEOF // i dont need you anymore :(

#endif
