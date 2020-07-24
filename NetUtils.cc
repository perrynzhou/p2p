/*************************************************************************
  > File Name: NetUtils.cc
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: Friday, July 24, 2020 PM03:29:13 HKT
 ************************************************************************/

#include "NetUtils.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
int demo::TcpSocket(const std::string& addr,int port,int backlog) {
  int fd = socket(PF_INET, SOCK_DGRAM, 0);
  struct sockaddr_in serveraddr;
  serveraddr.sin_family = AF_INET;
  inet_pton(AF_INET, addr.c_str(), &serveraddr.sin_addr);
  serveraddr.sin_port = htons(port);
  bind(fd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
  listen(fd,backlog);
  return fd;
}
int demo::UdpSocket(const std::string& addr,int port) {
   int fd = socket(PF_INET, SOCK_DGRAM, 0);
  struct sockaddr_in serveraddr;
  serveraddr.sin_family = AF_INET;
  inet_pton(AF_INET, addr.c_str(), &serveraddr.sin_addr);
  serveraddr.sin_port = htons(port);
  return fd;
}
 