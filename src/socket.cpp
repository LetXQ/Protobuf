#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#include "socket.h"
#include "notice.h"
#include "proto/user.pb.h"

constexpr unsigned int MAX_BUF_SIZE = 128;

Socket::Socket(int port, const std::string &ip)
    : m_port(port)
    , m_ip(ip)
{

}

Socket::~Socket()
{
    this->Reset();
}

int Socket::Init()
{
    int ret = 0;
    do{
        if (m_port < 2000 || m_ip.empty())
        {
            ret = ERR_PORT_OR_IP;
            break;
        }

        ret = this->InitSockFd();
        if (ret != 0)
            break;

        ret = this->DoInit();
        if (ret != 0)
            break;
    }while(0);
    return ret;
}

int Socket::InitSockFd()
{
    m_sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (m_sockfd == -1)
        return ERR_SOCKET_FAILED;

    int reuseaddr_on = 1;
    int ret = setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR,
                         &reuseaddr_on, sizeof(reuseaddr_on));
    if (ret == -1)
        return ERR_SOCKOPT_FAILED;
    return 0;
}

void Socket::Reset()
{
    close(m_sockfd);
    m_sockfd = -1;
    m_port = 0;
    m_ip = "";
}

ServSocket::ServSocket(int port, const std::string &ip, int backlog)
    : Socket(port, ip)
    , m_backlog(backlog)
{

}

int ServSocket::DoInit()
{
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(m_port);
    inet_pton(AF_INET, m_ip.c_str(), &serv_addr.sin_addr);

    int ret = bind(m_sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    if (-1 == ret)
    {
        return ERR_BIND_FAILED;
    }

    ret = listen(m_sockfd, m_backlog);
    if (-1 == ret)
        return ERR_LISTEN_FAILED;

    return 0;
}

void ServSocket::Run()
{
    struct sockaddr_in clnt_addr;
    while (true) {
       std::cout << "Watting Client: \n";
       socklen_t clnt_len =sizeof(clnt_addr);
       int clnt_fd = accept(m_sockfd, (struct sockaddr*)&clnt_addr,
                            &clnt_len);
       if (clnt_fd < 0)
           continue;

       int recv_size = 0;
       char buf[MAX_BUF_SIZE];
       memset(buf, 0, MAX_BUF_SIZE);
       recv_size = recv(clnt_fd, buf, MAX_BUF_SIZE, 0);
       std::string recv_data(buf, recv_size);
       std::cout << "From Client: " << inet_ntoa(clnt_addr.sin_addr) << std::endl;
       std::cout << "Clinett Msg: " << recv_data << std::endl;
       if ("GET" == recv_data)
       {
           std::cout << "Correct operator\n";
           User::UserInfo user;
           user.set_uid(9999);
           user.set_age(18);
           user.set_name("Lucifer");
           user.set_job(User::UserInfo_Job_TEACHER);

           std::string send_data;
           user.SerializeToString(&send_data);
           send(clnt_fd, send_data.c_str(), send_data.length(), 0);
       }
       else
       {
           std::cout << "Not Correct operator\n";
           std::string send_data("Not correct operator\n");
           send(clnt_fd, send_data.c_str(), send_data.length(), 0);
       }
       close(clnt_fd);
    }
}


ClntSocket::ClntSocket(int port, const std::string &ip)
    : Socket(port, ip)
{

}

void ClntSocket::Run()
{
    std::string send_data("GET");
    int send_bytes = send(m_sockfd, send_data.c_str(), send_data.length(), 0);
    if (send_bytes != send_data.length())
    {
        std::cout << "Send Error\n";
        return;
    }
    char buf[MAX_BUF_SIZE];
    memset(buf, 0, MAX_BUF_SIZE);

    int num_bytes = recv(m_sockfd, buf, MAX_BUF_SIZE, 0);
    std::string recv_data(buf, num_bytes);
    User::UserInfo parse_obj;
    bool b_ret = parse_obj.ParseFromString(recv_data);
    if (!b_ret)
    {
        std::cout << "Not protobuf msg!\n";
        std::cout << "Msg: " << recv_data << std::endl;
    }
    else
    {
        std::cout << "Is protobuf msg: \n";
        std::cout << "UserInfo:\n";
        std::cout << "ID: " << parse_obj.uid() << ", age: " << parse_obj.age() << ", name: " << parse_obj.name();
        switch (parse_obj.job()) {
        case 0:
            std::cout << ", job is teacher\n";
            break;
        case 1:
            std::cout << ", job is doctor\n";
            break;
        case 2:
            std::cout << ", job is police\n";
            break;
        default:
            break;
        }
    }
}

int ClntSocket::DoInit()
{
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(m_port);
    inet_pton(AF_INET, m_ip.c_str(), &serv_addr.sin_addr);

    int ret = connect(m_sockfd, (struct sockaddr*)&serv_addr,
                      sizeof(serv_addr));
    if (-1 == ret)
        return ERR_CONNECT_FAILED;
    return 0;
}
