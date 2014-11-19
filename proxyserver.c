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

// todo:
void setopt() {
}

int main(int argc, char**argv) {
  const int recvfd = socket(AF_INET, SOCK_STREAM, 0);
  if (recvfd < 0)
    error("ERROR opening socket");

  const int optval = 1;
  if (setsockopt(recvfd, SOL_SOCKET, SO_REUSEADDR,
                 (const void *)&optval, sizeof(int)) < 0)
    error("ERROR on setting reuseaddr");

  if (setsockopt(recvfd, IPPROTO_TCP, TCP_NODELAY,
                 (const void*)&optval, sizeof(int)) < 0)
    error("ERROR on setting nodelay");

  if (setsockopt(recvfd, SOL_SOCKET, SO_KEEPALIVE,
                 (const void*)&optval, sizeof(int)) < 0)
    error("ERROR on setting keepalive");

  // const int port = 1433;                // default port for ms sql server
  const int port = 5432;
  struct sockaddr_in servaddr;
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(port);
  if (bind(recvfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    error("ERROR on binding");

  const int maxconn = 1024;
  if (listen(recvfd, maxconn) < 0)
    error("ERROR on listen");

  struct sockaddr_in cliaddr;
  socklen_t clilen = sizeof(cliaddr);
  // for(;;)
  {
    const int connfd = accept(recvfd, (struct sockaddr *)&cliaddr, &clilen); // accept is blocking
    if (connfd < 0)
      error("ERROR on accept");

    struct hostent *hostp = gethostbyaddr((const char *)&cliaddr.sin_addr.s_addr,
                                          sizeof(cliaddr.sin_addr.s_addr), AF_INET);
    if (hostp == NULL)
      error("ERROR on gethostbyaddr");

    char* hostaddrp = inet_ntoa(cliaddr.sin_addr);
    if (hostaddrp == NULL)
      error("ERROR on inet_ntoa\n");
    printf("server established connection with %s (%s)\n", hostp->h_name, hostaddrp);

    // open a new socket to connect ms sql server
    const int sendfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sendfd < 0) 
      error("ERROR opening socket");
    struct sockaddr_in sendaddr;
    bzero(&sendaddr, sizeof(sendaddr));
    sendaddr.sin_family = AF_INET;
    sendaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    const int actport = port + 1;
    sendaddr.sin_port = htons(actport);    // port to connect actual sql server
    if (connect(sendfd, (struct sockaddr *) &sendaddr,sizeof(sendaddr)) < 0) 
      error("ERROR connecting");

    char buf[1024];
    bzero(buf, 1024);

    for (;;) {
      int n = read(connfd, buf, 1024);
      if (n > 0) {
        printf("server received %d bytes: %s\n", n, buf);
        const int m = write(connfd, buf, strlen(buf));
        if (m < 0)
          error("ERROR writing to socket");

        // const int m = write(sendfd, buf, strlen(buf));
        // if (m < 0)
        //   error("ERROR writing to socket");

        // bzero(buf, 1024);
        // const int j = read(sendfd, buf, 1024);
        // if (j < 0)
        //   error("ERROR reading from socket");
        // const int k = write(connfd, buf, strlen(buf));
        // if (k < 0)
        //   error("ERROR writing to socket");

      }

      // close(sendfd);
    }

    close(connfd);
  }

  return 0;
}
