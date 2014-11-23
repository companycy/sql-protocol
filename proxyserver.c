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

const int mssql_port = 1433;
const int pg_port = 5432;
const int mysql_port = 3306;

const char mbp_ip[] = "192.168.31.202";   // host ip
const char local_ip[] = "192.168.31.133"; // on ubuntu vm
const char* pg_ip = local_ip;             // on ubuntu vm
const char* mysql_ip = local_ip;          // on ubuntu vm
const char mssql_ip[] = "192.168.31.184"; // on win7 vm

void error(char *msg) {
  perror(msg);
  exit(1);
}

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

int get_sendfd(const char* sqlserver_ip, const int sqlserver_port) {
  const int sendfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sendfd < 0) {
    error("ERROR opening socket");
  } else if (!sqlserver_ip || *sqlserver_ip == '\0') {
    error("ERROR on sql server ip");
  } else {
    // printf("new socket %d\n", sendfd);
    setopt(sendfd);
  }

  struct sockaddr_in sendaddr;
  bzero(&sendaddr, sizeof(sendaddr));
  sendaddr.sin_family = AF_INET;
  sendaddr.sin_addr.s_addr = inet_addr(sqlserver_ip);
  sendaddr.sin_port = htons(sqlserver_port);
  if (connect(sendfd, (struct sockaddr *) &sendaddr,sizeof(sendaddr)) < 0) 
    error("ERROR connecting");
  else
    printf("connected to backend sql server from port: %d\n", sqlserver_port);

  return sendfd;
}

int main(int argc, char**argv) {
  // config here
  const int fe_port = mssql_port;  // accept connection from driver
  const char* be_ip = mssql_ip;    // sql server ip
  const int be_port = mssql_port;

  const int socketfd = socket(AF_INET, SOCK_STREAM, 0);
  if (socketfd < 0) {
    error("ERROR opening socket");
  } else {
    // printf("new socket %d\n", socketfd);
    setopt(socketfd);
  }

  struct sockaddr_in servaddr;
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(fe_port);
  servaddr.sin_family = AF_INET;
  if (bind(socketfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    error("ERROR on binding");

  const int maxconn = 1024;
  if (listen(socketfd, maxconn) < 0)
    error("ERROR on listen");

  struct sockaddr_in cliaddr;
  socklen_t clilen = sizeof(cliaddr);
  // for(;;)
  {
    const int connfd = accept(socketfd, (struct sockaddr *)&cliaddr, &clilen); // accept is blocking
    if (connfd < 0) {
      error("ERROR on accept");
    } else {
      // printf("new accepted fd %d\n", connfd);
    }

    struct hostent *hostp = gethostbyaddr((const char *)&cliaddr.sin_addr.s_addr,
                                          sizeof(cliaddr.sin_addr.s_addr), AF_INET);
    if (hostp == NULL)
      error("ERROR on gethostbyaddr");

    const char* hostaddrp = inet_ntoa(cliaddr.sin_addr);
    if (hostaddrp == NULL)
      error("ERROR on inet_ntoa\n");
    printf("server established connection with %s (%s) from port: %d\n", hostp->h_name, hostaddrp, fe_port);

    const int sendfd = get_sendfd(be_ip, be_port); // new socket to connect ms sql server

    char buf[1024];
    bzero(buf, sizeof(buf));

    for (;;) {
      const int n = read(connfd, buf, sizeof(buf));
      if (n > 0) {
        printf("from frontend:  %d bytes\n", n);

        for (int i = 0; i < n; ++i) {
          printf("%c", *(buf + i));
        }
        printf("\n");


        if (buf[0] == 0x01) {  // from tds protocol v7, it's a query packet
          printf("\n");
          printf("\n");
          // char query[1024];
          // bzero(query, sizeof(query));
          // strncpy(query, (const char*)(buf + 1), n - 1);
          printf("one more packet, and first byte: %d, left bytes: %s\n", buf[0], buf + 1);
        }

        const int m = write(sendfd, buf, n);
        if (m < 0)
          error("ERROR writing to socket");
        else
          printf("write to backend %d bytes, total %d bytes\n", m, n);

        bzero(buf, sizeof(buf));
        const int j = read(sendfd, buf, sizeof(buf));
        if (j < 0)
          error("ERROR reading from socket");
        else
          printf("read from backend bytes:  %d\n", j);

        const int k = write(connfd, buf, j);
        if (k < 0)
          error("ERROR writing to socket");
        else
          printf("write to frontend bytes: %d\n", k);

      }

      // close(sendfd);
    }

    close(connfd);
  }

  return 0;
}
