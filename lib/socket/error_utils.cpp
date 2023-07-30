#include <cerrno>
#include <cstring>
#include <string>

#include "socket/error_utils.hpp"

namespace icysock_errors {

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

        case errc::bad_socket:
          return std::string("Bad Socket: ") +
                 std::string(std::strerror(errno));

        case errc::getaddrinfo_failure:
          return std::string("getaddrinfo() failed");

        case errc::empty_addrinfo:
          return std::string("addrinfo list structure is empty");

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
SocketInitError::SocketInitError(const std::string& what_arg)
  : what_string(what_arg)
{
}

SocketInitError::SocketInitError(const char* what_arg)
  : what_string(what_arg)
{
}

SocketInitError::SocketInitError(const errc ec)
  : ecode(make_error_code(ec))
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
}

SocketInitError&
SocketInitError::operator=(const SocketInitError& other) noexcept
{
  this->what_string = other.what_string;
  return *this;
}

const char*
SocketInitError::what() const noexcept
{
  return what_string.c_str();
}

} // namespace icysock_errors

/* specialisation for is_error_code_enum for our own enum type */
namespace std {
template<>
struct is_error_code_enum<icysock_errors::errc> : true_type
{};
}