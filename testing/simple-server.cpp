/*
** server.c -- a stream socket server demo
*/

#include "socket/socket.hpp"

#include <sys/wait.h>

#define PORT "3490" // the port users will be connecting to

#define BACKLOG 10 // how many pending connections queue will hold

using namespace BetterSocket;

void
sigchld_handler(int s)
{
  // waitpid() might overwrite errno, so we save and restore it:
  int saved_errno = errno;

  while (waitpid(-1, NULL, WNOHANG) > 0)
    ;

  errno = saved_errno;
}

int
main(void)
{
  BSocket new_fd = BSocket(); // listen on sock_fd, new connection on new_fd
  SockaddrWrapper their_addr; // connector's address information
  struct sigaction sa;

  SocketHint hint(
    IpVersion::vAny, SockKind::Stream, SockFlags::UseHostIP, IpProtocol::TCP);

  BSocket sockfd(hint, PORT, "");
  sockfd.bind();
  sockfd.listen(BACKLOG);

  sa.sa_handler = sigchld_handler; // reap all dead processes
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  if (sigaction(SIGCHLD, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  printf("server: waiting for connections...\n");

  while (1) { // main accept() loop
    new_fd = sockfd.accept(their_addr);
    if (new_fd == -1) {
      perror("accept");
      continue;
    }

    printf("server: got connection from %s\n", their_addr.getIP().c_str());

    if (!fork()) {    // this is the child process
      sockfd.close(); // child doesn't need the listener
      if (new_fd.send("Hello, world!") == -1)
        perror("send");
      new_fd.close();
      exit(0);
    }
    new_fd.close(); // parent doesn't need this
  }

  return 0;
}