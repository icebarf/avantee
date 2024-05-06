#ifndef ICETEA_ERROR_UTILS_H
#define ICETEA_ERROR_UTILS_H

#include <exception>
#include <string>
#include <system_error>

namespace SockErrors {
/* we want the error code to be truthy
 * eg: if (error) then_do_whatever();
 */
enum class errc
{
  success = 0,
  accept_failure,
  bad_addrinfolist,
  bad_socket,
  bind_failure,
  connect_failure,
  getaddrinfo_failure,
  ipfamily_not_set,
  listen_failure,
  receive_failure,
  receive_from_failure,
  send_failure,
  sendto_failure,
  setsockopt_failure,
  shutdown_failure,
};

std::error_code
make_error_code(errc e) noexcept;

/* higher level exceptions
 * should be thrown by the server/client code
 * with use of primitives */

class SocketInitError : public std::exception
{
private:
  std::error_code ecode;
  std::string what_string;

public:
  explicit SocketInitError(const std::string& what_arg);
  explicit SocketInitError(const char* what_arg);
  SocketInitError(const SocketInitError& other);
  SocketInitError(const errc ec);
  SocketInitError(const errc ec, std::string what_arg);
  SocketInitError(const errc ec, const char* what_arg);

  SocketInitError& operator=(const SocketInitError& other) noexcept;

  const char* what() const noexcept override;
  errc whatErrc() const noexcept;
};

class APIError : public std::exception
{
private:
  std::error_code ecode;
  std::string what_string;

public:
  explicit APIError(const std::string& what_arg);
  explicit APIError(const char* what_arg);
  APIError(const APIError& other);
  APIError(const errc ec);
  APIError(const errc ec, std::string what_arg);
  APIError(const errc ec, const char* what_arg);

  APIError& operator=(const APIError& other) noexcept;

  const char* what() const noexcept override;
  errc whatErrc() const noexcept;
};

}

#endif
