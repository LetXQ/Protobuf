#ifndef SOCKET_H
#define SOCKET_H
#include <iostream>
#include <string.h>
#include <map>

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

class ServSocket;
struct pthread_args_t
{
    int new_fd = -1;
    ServSocket* p_serv = nullptr;
};

class ServSocket : public Socket
{
private:
    int m_backlog = 10;

public:
    struct clnt_info_t
    {
        int count = 0;
        std::string ip_addr = "";
    };

    using clnt_addr_map_t = std::map<int, clnt_info_t>;
    clnt_addr_map_t m_clnt_map;

public:
    ServSocket(int port, const std::string& ip, int backlog = 10);
    void Run() override;
    void ResetTickCount(int fd);

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
