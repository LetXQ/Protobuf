#ifndef SOCKET_H
#define SOCKET_H
#include <iostream>
#include <string.h>

class Socket
{
protected:
    int m_sockfd = -1;
    int m_port = 0;
    std::string m_ip = "";
public:
    Socket(int port, const std::string& ip);
    virtual ~Socket();
    int Init();
    virtual void Run() = 0;
private:
    int InitSockFd();
    void Reset();
protected:
    virtual int DoInit() = 0;
};


class ServSocket : public Socket
{
private:
    int m_backlog = 10;
public:
    ServSocket(int port, const std::string& ip, int backlog = 10);
    void Run() override;
private:
    int DoInit() override;
};

class ClntSocket: public Socket
{
public:
    ClntSocket(int port, const std::string& ip);
    void Run() override;
private:
    int DoInit() override;
};

#endif // SOCKET_H
