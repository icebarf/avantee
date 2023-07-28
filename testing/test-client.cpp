#include "socket/generic_sockets.hpp"

#include <cstdio>

#ifndef ICY_ON_WINDOWS
#include <string.h>
#define ZeroMemory(ptr, n) memset(ptr, 0, n);
#endif

int
main(int argc, char** argv)
{
  icysock::SOCKDATA data = icysock::init();

  (void)data;
  (void)argc;

  struct addrinfo *result = nullptr, *ptr = nullptr;
  struct addrinfo hints;
  ZeroMemory(&hints, sizeof(hints));
  hints.ai_family = AF_UNSPEC, hints.ai_socktype = SOCK_STREAM,
  hints.ai_protocol = IPPROTO_TCP;

#define PORT "6969"

  long long iResult = getaddrinfo(argv[1], PORT, &hints, &result);
  if (iResult != 0) {
    fprintf(
      stderr, "test: client getaddrinfo failed with code: %lld\n", iResult);
    icysock::terminate();
    return 1;
  }

  /* Obtain socket */
  icysock::icy_socket sock = BAD_SOCKET;
  ptr = result;
  sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
  if (sock == BAD_SOCKET) {
    fprintf(stderr, "test: Unable to obtain a socket\n");
    perror("test: ");
    freeaddrinfo(result);
    icysock::terminate();
    return 1;
  }

  /* connect to socket */
  iResult = connect(sock, ptr->ai_addr, (int)ptr->ai_addrlen);
  if (iResult == SOCK_ERR) {
    fprintf(stderr, "test: Unable to connect to socket\n");
    perror("test: ");
    icysock::close_socket(sock);
    freeaddrinfo(result);
    icysock::terminate();
    return 1;
  }

#define BUFLEN 512
  int recvbuflen = BUFLEN;

  const char* sendbuf = "this is a test";
  char recvbuf[BUFLEN] = {};

  iResult = send(sock, sendbuf, (int)strlen(sendbuf), 0);
  if (iResult == SOCK_ERR) {
    fprintf(stderr, "test: send() failed \n");
    perror("test: ");
    icysock::close_socket(sock);
    icysock::terminate();
    return 1;
  }

  printf("Bytes sent: %lld\n", iResult);

  iResult = shutdown(sock, SHUT_WR);
  if (iResult == SOCK_ERR) {
    fprintf(stderr, "test: shutdown() failed \n");
    perror("test: ");
    icysock::close_socket(sock);
    icysock::terminate();
    return 1;
  }

  do {
    iResult = recv(sock, recvbuf, recvbuflen, 0);
    if (iResult > 0)
      fprintf(stdout, "Bytes receieved: %lld\n", iResult);
    else if (iResult == 0)
      fprintf(stdout, "Connection closed\n");
    else {
      fprintf(stderr, "test: recv failed\n");
      perror("test");
    }
  } while (iResult > 0);

  icysock::close_socket(sock);
  icysock::terminate();
  return 0;
}