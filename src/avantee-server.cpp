#include <string_view>

#include "socket/error_utils.hpp"
#include "socket/sockets.hpp"

using namespace BetterSockets;

constexpr std::string_view TFTP_PORT{ "69" };

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
    } catch (const icysock::errors::APIError& e) {
      tftp_listener.try_next();
      continue;
    }
    break;
  }
}