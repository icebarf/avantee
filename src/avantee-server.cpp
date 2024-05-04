#include <array>
#include <cstddef>
#include <string_view>

#include "socket/error_utils.hpp"
#include "socket/sockets.hpp"

using namespace BetterSocket;
using BetterSocket::fd_type;

enum {
  MAX_PACKET_LEN = 516,
};

constexpr std::string_view TFTP_PORT{ "69" };

void
checked_try_next(BetterSocket::managed_socket& s)
{
  try {
    s.try_next();
  } catch (const sock_errors::APIError& ex) {
    fprintf(stderr,
            "Failure while moving to next address info node: %s\n",
            ex.what());
    exit(EXIT_FAILURE);
  }
}

void
watch(BetterSocket::managed_socket& tftp_listener)
{

  /* prepare for multiplexing */
  fdset_wrapper<fd_type::READ> master_readset{};
  master_readset.append(tftp_listener);
  multiplexer params(5, 0, tftp_listener.socket_handle);

  /* for receive_from to store information about client */
  struct sockaddr_storage client_info;
  std::array<std::byte, MAX_PACKET_LEN> packet;
  fdset_wrapper<fd_type::READ> readset{};

  while (true) {
    readset = master_readset;
    if (select(params.watching_over + 1,
               &readset.set,
               nullptr,
               nullptr,
               &params.timeout) == SOCK_ERR) {
      perror("select()");
      exit(1);
    }

    for (int i = 0; i <= params.watching_over; i++) {
      if (readset.isset(i)) {
        // new connection
        if (i == tftp_listener) {
          BetterSocket::size client_sz = sizeof client_info;
          tftp_listener.receive_from(
            packet.data(),
            packet.size(),
            reinterpret_cast<struct sockaddr*>(&client_info),
            &client_sz);

          // process receieved packet
        } // if i == tftp_listener
      } // if readset.isset(i)

      fprintf(stdout, "no connection\n");
    } // for (i = 0; i <= watching over; i++)
  }
}

int
main()
{
  auto tftp_listener =
    BetterSocket::managed_socket(socket_hint(ip_version::IpvAny,
                                              sock_kind::DGRAM,
                                              sock_flags::USE_HOST_IP,
                                              ip_protocol::UDP),
                                  TFTP_PORT.data());

  // bind to the first address we can
  for (;;) {
    try {
      tftp_listener.binds();
    } catch (const sock_errors::APIError& e) {
      fprintf(stderr, "Could not bind to host on port 69: %s\n", e.what());
      fprintf(stdout,
              "Trying to bind with the next available address info node.\n");

      checked_try_next(tftp_listener);
      continue;
    }
    break;
  }

  watch(tftp_listener);
}
