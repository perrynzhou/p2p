/*************************************************************************
  > File Name: NetUtils.h
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: Friday, July 24, 2020 PM03:29:09 HKT
 ************************************************************************/

#ifndef _NETUTILS_H
#define _NETUTILS_H
#include <string>
namespace demo {
  int TcpSocket(const std::string& addr,int port,int backlog=1024);
  int UdpSocket(const std::string& addr,int port);
};
#endif
