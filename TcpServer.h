/*************************************************************************
    > File Name: TcpServer.h
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: Sun 26 Jul 2020 08:04:29 PM CST
 ************************************************************************/

#ifndef _TCPSERVER_H
#define _TCPSERVER_H
namespace p2p
{

  class TcpServer
  {
    public:
      TcpServer()
    private:
    int m_efd;
    int m_sfd;
    int m_max_connections;
    struct epoll_event m_event;
  };
}; // namespace p2p
#endif
