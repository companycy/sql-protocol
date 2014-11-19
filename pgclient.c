/* Proxy client */

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

  const int port = 5433;                // pgsql port
  struct sockaddr_in servaddr;
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(port);
  if (connect(socketfd, (struct sockaddr *)&servaddr,sizeof(servaddr)) < 0) 
    error("ERROR on connecting");

  const short major_version = htons(3);
  const short minor_version = htons(0);
  // const char dbname_key[] = "dbname=";
  // const char dbname_value[] = "postgres";
  const char user_key[] = "user";
  const char user_value[] = "postgres";
  const int packetsize = 4                      // packetsize
                         + 2 + 2                // major_version + minor_version
                         // + sizeof(dbname_key) + sizeof(dbname_value) // dbname
                         + sizeof(user_key) + sizeof(user_value)     // user
                         + 1;                                        // ending 0

  char buf[1024] = "";
  bzero(buf, 1024);

  int offset = 0;
  const int n_packetsize = htonl(packetsize);
  memcpy(buf, &(n_packetsize), sizeof(packetsize));
  offset += sizeof(packetsize);
  memcpy(buf + offset, &(major_version), sizeof(major_version));
  offset += sizeof(major_version);
  memcpy(buf + offset, &(minor_version), sizeof(minor_version));
  offset += sizeof(minor_version);

  // memcpy(buf + offset, &dbname_key, sizeof(dbname_key));
  // offset += sizeof(dbname_key);
  // memcpy(buf + offset, &dbname_value, sizeof(dbname_value));
  // offset += sizeof(dbname_value);

  memcpy(buf + offset, &user_key, sizeof(user_key));
  offset += sizeof(user_key);
  memcpy(buf + offset, &user_value, sizeof(user_value));
  offset += sizeof(user_value);
  buf[packetsize] = 0;
  offset += 1;

  const int n = write(socketfd, buf, packetsize);
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
