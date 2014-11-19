/* Proxy server */

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>

void error(char *msg) {
  perror(msg);
  exit(1);
}

int main(int argc, char**argv) {
  const int socketfd = socket(AF_INET, SOCK_STREAM, 0);
  if (socketfd < 0)
    error("ERROR on opening socket");

  const int optval = 1;
  if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR,
                 (const void *)&optval, sizeof(int)) < 0)
    error("ERROR on setting reuseaddr");

  if (setsockopt(socketfd, IPPROTO_TCP, TCP_NODELAY,
                 (const void*)&optval, sizeof(int)) < 0)
    error("ERROR on setting nodelay");

  if (setsockopt(socketfd, SOL_SOCKET, SO_KEEPALIVE,
                 (const void*)&optval, sizeof(int)) < 0)
    error("ERROR on setting keepalive");

  // const int port = 1433;                // default port for ms sql server
  const int port = 5433;
  struct sockaddr_in servaddr;
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(port);
  if (connect(socketfd, (struct sockaddr *)&servaddr,sizeof(servaddr)) < 0) 
    error("ERROR on connecting");

  char buf[1024] = "hello world";
  const int n = write(socketfd, buf, strlen(buf));
  if (n < 0)
    error("ERROR on writing to socket");
  else
    printf("write %d bytes\n", n);

  bzero(buf, 1024);
  const int m = read(socketfd, buf, 1024);
  if (m < 0)
    error("ERROR on reading from socket");
  else
    printf("read from socket: %s, bytes: %d\n", buf, m);

  close(socketfd);
  return 0;
}
