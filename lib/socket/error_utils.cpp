#include <cerrno>
#include <cstring>
#include <string>

#include "socket/error_utils.hpp"

namespace SockErrors {

std::error_code
make_error_code(errc e) noexcept
{
  static const struct error_category : std::error_category
  {
    virtual const char* name() const noexcept override
    {
      return "Berkely Socket Abstraction";
    }

    virtual std::string message(int condition) const override
    {
      switch (static_cast<errc>(condition)) {
        case errc::success:
          return "Success";

        case errc::accept_failure:
          return std::string("accept() failed");

        case errc::bad_socket:
          return std::string("Bad Socket: ") +
                 std::string(std::strerror(errno));

        case errc::bad_addrinfolist:
          return std::string("addrinfo list structure is bad");

        case errc::bind_failure:
          return std::string("bind() failed");

        case errc::connect_failure:
          return std::string("connect() failed");

        case errc::getaddrinfo_failure:
          return std::string("getaddrinfo() failed");

        case errc::listen_failure:
          return std::string("listen() failed");

        case errc::ipfamily_not_set:
          return std::string("SockaddrWrapper::IsSetIPCalled is false.");

        case errc::receive_failure:
          return std::string("recv() failed");

        case errc::receive_from_failure:
          return std::string("recvfrom() failed");

        case errc::send_failure:
          return std::string("send() failed");

        case errc::sendto_failure:
          return std::string("sendto() failed");

        case errc::setsockopt_failure:
          return std::string("setsockopt() failed");

        case errc::shutdown_failure:
          return std::string("shutdown() failed");

        default:
          return "Unknown error";
      }
    }

    virtual std::error_condition default_error_condition(
      int code) const noexcept override
    {
      if (code)
        return std::errc::resource_unavailable_try_again;
      return {};
    }
  } category;

  return std::error_code(static_cast<int>(e), category);
}

/* Exceptions */

/* SocketInitError*/

SocketInitError::SocketInitError(const std::string& what_arg)
  : ecode()
  , what_string(what_arg)
{
}

SocketInitError::SocketInitError(const char* what_arg)
  : ecode()
  , what_string(what_arg)
{
}

SocketInitError::SocketInitError(const errc ec)
  : ecode(make_error_code(ec))
  , what_string()
{
}

SocketInitError::SocketInitError(const errc ec, std::string what_arg)
  : ecode(make_error_code(ec))
  , what_string(what_arg)
{
}

SocketInitError::SocketInitError(const errc ec, const char* what_arg)
  : ecode(make_error_code(ec))
  , what_string(what_arg)
{
}

SocketInitError::SocketInitError(const SocketInitError& other)
{
  this->what_string = other.what_string;
  this->ecode = other.ecode;
}

SocketInitError&
SocketInitError::operator=(const SocketInitError& other) noexcept
{
  this->what_string = other.what_string;
  this->ecode = other.ecode;
  return *this;
}

const char*
SocketInitError::what() const noexcept
{
  return what_string.c_str();
}

errc
SocketInitError::whatErrc() const noexcept
{
  return static_cast<errc>(ecode.value());
}

/* APIError */
APIError::APIError(const std::string& what_arg)
  : ecode()
  , what_string(what_arg)
{
}

APIError::APIError(const char* what_arg)
  : ecode()
  , what_string(what_arg)
{
}

APIError::APIError(const errc ec)
  : ecode(make_error_code(ec))
  , what_string()
{
}

APIError::APIError(const errc ec, std::string what_arg)
  : ecode(make_error_code(ec))
  , what_string(what_arg)
{
}

APIError::APIError(const errc ec, const char* what_arg)
  : ecode(make_error_code(ec))
  , what_string(what_arg)
{
}

APIError::APIError(const APIError& other)
{
  this->what_string = other.what_string;
  this->ecode = other.ecode;
}

APIError&
APIError::operator=(const APIError& other) noexcept
{
  this->what_string = other.what_string;
  this->ecode = other.ecode;
  return *this;
}

const char*
APIError::what() const noexcept
{
  return what_string.c_str();
}

errc
APIError::whatErrc() const noexcept
{
  return static_cast<errc>(ecode.value());
}

} // namespace icysock_errors

/* specialisation for is_error_code_enum for our own enum type */
namespace std {
template<>
struct is_error_code_enum<SockErrors::errc> : true_type
{};
}
