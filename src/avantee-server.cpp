#include <array>
#include <cstddef>
#include <string_view>

#include "socket/error_utils.hpp"
#include "socket/sockets.hpp"

using namespace BetterSockets;
using namespace icysock::errors;
using BetterSockets::fd_type;

constexpr std::string_view TFTP_PORT{ "69" };
constexpr int MAX_PACKET_BYTES{ 516 };

void
checked_try_next(BetterSockets::managed_socket& s)
{
  // we do this because we want proper message propagation to the user.
  try {
    s.try_next();
  } catch (const APIError& ex) {
    fprintf(stderr,
            "Failure while moving to next address info node: %s\n",
            ex.what());
    exit(EXIT_FAILURE);
  }
}

void
watch(BetterSockets::managed_socket& tftp_listener)
{

  /* prepare for multiplexing */
  fd_set_wrapper<fd_type::READ> master_readset;
  master_readset.append(tftp_listener);
  multiplexer params(5, 0, tftp_listener.socket_handle);

  /* for receive_from to store information about client */
  struct sockaddr_storage client_info;
  std::array<std::byte, MAX_PACKET_BYTES> packet;
  fd_set readset;
  FD_ZERO(&readset);

  while (true) {
    readset = master_readset.set;
    if (select(params.watching_over + 1,
               &readset,
               nullptr,
               nullptr,
               &params.timeout) == SOCK_ERR) {
      perror("select()");
      exit(1);
    }

    for (int i = 0; i <= params.watching_over; i++) {
      if (FD_ISSET(i, &readset)) {
        // new connection
        if (i == tftp_listener) {
          icysock::size client_sz = sizeof client_info;
          tftp_listener.receive_from(
            packet.data(),
            packet.size(),
            reinterpret_cast<struct sockaddr*>(&client_info),
            &client_sz);

          // process receieved packet
        } // if i == tftp_listener
      }   // if FD_ISSET(i, &readset)

      fprintf(stdout, "no connection\n");
    } // for (i = 0; i <= watching over; i++)
  }
}

int
main()
{
  auto tftp_listener =
    BetterSockets::managed_socket(socket_hint(ip_version::IpvAny,
                                              sock_kind::DGRAM,
                                              sock_flags::USE_HOST_IP,
                                              ip_protocol::UDP),
                                  TFTP_PORT.data());

  // bind to the first address we can
  for (;;) {
    try {
      tftp_listener.binds();
    } catch (const APIError& e) {
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