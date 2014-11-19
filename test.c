
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

int main() {
  const int version = htonl(3);
  const char key[] = "dbname";
  const char value[] = "postgres";
  const int packetsize = htonl(4 + 4 + 1 + sizeof(key) + sizeof(value));

  // char buf[1024] = "";
  // bzero(buf, 1024);
  // memcpy(buf, &packetsize, sizeof(int));
  // memcpy(buf + 4, &version, sizeof(int));
  // buf[8] = '\0';
  // memcpy(buf + 9, &key, sizeof(key));
  // memcpy(buf + 9 + sizeof(key), &value, sizeof(value));
  // buf[packetsize] = '\0';


  char buf[1024] = "";
  bzero(buf, 1024);
  memcpy(buf, &(packetsize), sizeof(packetsize));
  int offset = sizeof(packetsize);
  memcpy(buf + offset, &(version), sizeof(version));
  offset += sizeof(version);
  buf[8] = '\0';
  ++ offset;
  memcpy(buf + offset, &key, sizeof(key));
  offset += sizeof(key);
  memcpy(buf + offset, &value, sizeof(value));
  offset += sizeof(value);


  // printf("len: %d\n", packetsize);
  return 0;
}


