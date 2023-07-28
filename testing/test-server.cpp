#include "generic_sockets.hpp"

#include <cstdio>

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "6969"

#ifndef ICY_ON_WINDOWS
#include <string.h>
#define ZeroMemory(ptr, n) memset(ptr, 0, n);
#endif

int
main(void)
{
  icysock::SOCKDATA wsaData = icysock::init();
  (void)wsaData;
  long long iResult;

  icysock::icy_socket ListenSocket = BAD_SOCKET;
  icysock::icy_socket ClientSocket = BAD_SOCKET;

  struct addrinfo* result = NULL;
  struct addrinfo hints;
  ZeroMemory(&hints, sizeof(hints));
  hints.ai_flags = AI_PASSIVE, hints.ai_family = AF_INET,
  hints.ai_socktype = SOCK_STREAM, hints.ai_protocol = IPPROTO_TCP;

  long long iSendResult;
  char recvbuf[DEFAULT_BUFLEN];
  int recvbuflen = DEFAULT_BUFLEN;

  // Resolve the server address and port
  iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
  if (iResult != 0) {
    printf("getaddrinfo failed with error: %lld\n", iResult);

    icysock::terminate();
    return 1;
  }

  // Create a SOCKET for the server to listen for client connections.
  ListenSocket =
    socket(result->ai_family, result->ai_socktype, result->ai_protocol);
  if (ListenSocket == BAD_SOCKET) {
    printf("socket failed with error\n");
    perror("test");
    freeaddrinfo(result);
    icysock::terminate();
    return 1;
  }

  // Setup the TCP listening socket
  iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
  if (iResult == SOCK_ERR) {
    printf("bind failed with error: %lld\n", iResult);
    perror("test: ");
    freeaddrinfo(result);
    icysock::close_socket(ListenSocket);
    icysock::terminate();
    return 1;
  }

  freeaddrinfo(result);

  iResult = listen(ListenSocket, SOMAXCONN);
  if (iResult == SOCK_ERR) {
    printf("listen failed with error: %lld\n", iResult);
    perror("test: ");
    icysock::close_socket(ListenSocket);
    icysock::terminate();
    return 1;
  }

  // Accept a client socket
  ClientSocket = accept(ListenSocket, NULL, NULL);
  if (ClientSocket == INVALID_SOCKET) {
    printf("accept failed with error\n");
    perror("test");
    icysock::close_socket(ListenSocket);
    icysock::terminate();
    return 1;
  }

  // No longer need server socket
  icysock::close_socket(ListenSocket);

  // Receive until the peer shuts down the connection
  do {

    iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
    recvbuf[recvbuflen - 1] = '\0';
    if (iResult > 0) {
      printf("Bytes received: %lld\n", iResult);
      printf("Contents: %s\n", recvbuf);
      // Echo the buffer back to the sender
      iSendResult = send(ClientSocket, recvbuf, (int)iResult, 0);
      if (iSendResult == SOCK_ERR) {
        printf("send failed with error: %lld\n", iSendResult);
        perror("test: ");
        icysock::close_socket(ClientSocket);
        icysock::terminate();
        return 1;
      }
      printf("Bytes sent: %lld\n", iSendResult);
    } else if (iResult == 0)
      printf("Connection closing...\n");
    else {
      printf("recv failed with error: %lld\n", iResult);
      perror("test: ");
      icysock::close_socket(ClientSocket);
      icysock::terminate();
      return 1;
    }

  } while (iResult > 0);

  // shutdown the connection since we're done
  iResult = shutdown(ClientSocket, SHUT_WR);
  if (iResult == SOCK_ERR) {
    printf("shutdown failed with error: %lld\n", iResult);
    perror("test: ");
    icysock::close_socket(ClientSocket);
    icysock::terminate();
    return 1;
  }

  // cleanup
  icysock::close_socket(ClientSocket);
  icysock::terminate();

  return 0;
}