# Project Avantee

Avantee is a codename for my implementation of a TFTP client-server suite.

## Reason of existence

This is an exercise to work with C++'s abstraction capabilites, while also
learning about socket programming. I chose RFC 1350 because it is very
"trivial" to understand and implement.

## Details

The way this project was designed is listed as follows, with ascending order
of abstraction introduced. Everything is wrapped in a namespace.

### `<socket/generic_sockets.hpp>`

Generalises the differences between Windows and POSIX implementation of
Berkely Sockets. Provides
- signed and unsigned sign types
- structure zeroing routine
- init and cleanup routines for cross-platform compatibility
- bridge routines that have different names in implementations

### `<socket/sockets.hpp>`

Build upon that generalisation and introduce readable and meaningful names.
- Macros as `constexpr` variables
- Enumerations wrapped in `enum class` for type safety
- Wrapper classes that **_partially_** abstracts away the laborous C API.

### `<socket/error_utils.hpp>`

Provides meaningful errors and implements an exception type for error handling.

## My Thoughts

The project has many refactoring capabilities, such as using templates, better
error handling with alternative methods, improving the overall meaningfulness
of the wrapper classes, and covering more of the Berkely Sockets API.
Currently, the goal is to get things working and only cover part of the API
that will be useful in implementing a TFTP client-server.

As of writing this document, I have mostly worked on minimal abstraction
capabilities, and some testing with an example program. As more work is done,
this section will be updated.

## Resources Used

- [Microsoft Winsock2 Documentation](https://learn.microsoft.com/en-us/windows/win32/winsock/getting-started-with-winsock)
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/html/index-wide.html)
- [RFC 1350](https://web.archive.org/web/20240104152720/https://datatracker.ietf.org/doc/html/rfc1350)
- [cppreference](https://en.cppreference.org/w/c++)
