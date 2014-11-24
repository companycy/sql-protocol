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

// const int mssql_port = 1433;
// const int pg_port = 5432;
// const int mysql_port = 3306;

// const char mbp_ip[] = "192.168.31.202";   // host ip
// const char local_ip[] = "127.0.0.1";      // ubuntu vm
// const char* pg_ip = local_ip;             // ubuntu vm
// const char* mysql_ip = local_ip;          // ubuntu vm
// const char mssql_ip[] = "192.168.31.184"; // win7 vm

enum SqlserverType {
  MSSQL_SERVER = 0,
  PG_SERVER,
  MYSQL_SERVER
};

struct ServerInfo {
  int fe_port;
  char* be_ip;
  int be_port;
};

const struct ServerInfo server_infos[4] = {
  {1433, "192.168.31.184", 1433},       // mssql
  {5432, "127.0.0.1", 5433},            // pg
  {3306, "127.0.0.1", 3307},            // mysql
  {0, "", 0},
};

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
    // printf("new socket on %d, %s %d\n", sendfd, sqlserver_ip, sqlserver_port);
    setopt(sendfd);
  }

  struct sockaddr_in sendaddr;
  bzero(&sendaddr, sizeof(sendaddr));
  sendaddr.sin_family = AF_INET;
  // sendaddr.sin_addr.s_addr = inet_addr(sqlserver_ip);
  // printf("local: %d, inet: %d\n", htonl(INADDR_ANY), inet_addr("127.0.0.1"));
  sendaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  sendaddr.sin_port = htons(sqlserver_port);
  if (connect(sendfd, (struct sockaddr *) &sendaddr,sizeof(sendaddr)) < 0) 
    error("ERROR connecting");
  else
    printf("connected to backend sql server from port: %d\n", sqlserver_port);

  return sendfd;
}

void get_ms_sql(const char* buf, const int len) {
  for (int i = 0; i < len; ++i) {
    printf("%c", *(buf + i));
  }
  printf("\n");
}

void get_pg_sql(const char* buf, const int len) {
  for (int i = 0; i < len; ++i) {
    printf("%c", *(buf + i));
  }
  printf("\n");
}

void get_mysql_sql(const char* buf, const int len) {
}

void get_sql(const enum SqlserverType type, const char* buf, const int len) {
  switch (type) {
    case MSSQL_SERVER:
      if (buf[0] == 0x01) {             // a query packet based on tds protocol v7
        get_ms_sql(buf + 1, len);
      }
      break;
    case PG_SERVER:
      if (buf[0] == 'P') {              // a query starting with Parse based on pg v3
        get_pg_sql(buf + 1, len);
      }
      break;
    case MYSQL_SERVER:                  // todo:
      get_mysql_sql(buf + 1, len);
      break;
    default:
      printf("ERROR on server type\n");
      break;
  }
}

int main(int argc, char**argv) {
  // config here
  const enum SqlserverType type = PG_SERVER;

  const int idx = (int)type;
  const int fe_port = server_infos[idx].fe_port; // accept connection from driver
  const char* be_ip = server_infos[idx].be_ip;   // sql server ip
  const int be_port = server_infos[idx].be_port;

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

        // for (int i = 0; i < n; ++i) {
        //   printf("%c", *(buf + i));
        // }
        // printf("\n");

        get_sql(type, buf, n);

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
