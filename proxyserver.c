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
void setopt(const int recvfd) {
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
}

int main(int argc, char**argv) {
  const int recvfd = socket(AF_INET, SOCK_STREAM, 0);
  if (recvfd < 0) {
    error("ERROR opening socket");
  } else {
    // printf("new socket %d\n", recvfd);
    setopt(recvfd);
  }

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
    if (connfd < 0) {
      error("ERROR on accept");
    } else {
      // printf("new accepted fd %d\n", connfd);
    }

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
    if (sendfd < 0) {
      error("ERROR opening socket");
    } else {
      // printf("new socket %d\n", sendfd);
      setopt(sendfd);
    }

    struct sockaddr_in sendaddr;
    bzero(&sendaddr, sizeof(sendaddr));
    sendaddr.sin_family = AF_INET;
    sendaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    const int actport = port + 1;
    sendaddr.sin_port = htons(actport);    // port to connect actual sql server
    if (connect(sendfd, (struct sockaddr *) &sendaddr,sizeof(sendaddr)) < 0) 
      error("ERROR connecting");
    else
      printf("connected to backend sql server\n");

    char buf[1024];
    bzero(buf, sizeof(buf));

    for (;;) {
      const int n = read(connfd, buf, sizeof(buf));
      if (n > 0) {
        printf("server received %d bytes\n", n);

        // for (int i = 0; i < n; ++i) {
        //   printf("%d", *(buf + i));
        // }
        // printf("\n");

        const int m = write(sendfd, buf, n);
        if (m < 0)
          error("ERROR writing to socket");
        else
          printf("write %d bytes, total %d bytes\n", m, n);

        bzero(buf, sizeof(buf));
        const int j = read(sendfd, buf, sizeof(buf));
        if (j < 0)
          error("ERROR reading from socket");
        else
          printf("read from backend server bytes:  %d\n", j);

        const int k = write(connfd, buf, j);
        if (k < 0)
          error("ERROR writing to socket");
        else
          printf("write back to front end bytes: %d\n", k);

      }

      // close(sendfd);
    }

    close(connfd);
  }

  return 0;
}
