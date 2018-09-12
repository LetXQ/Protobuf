#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sstream>

#include "include/socket.h"
#include "include/notice.h"
#include "proto/user.pb.h"
#include "include/data_def.h"
#include "include/bytebuffer.h"

constexpr unsigned int MAX_BUF_SIZE = 1024;

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

void* HeartHandler(void* args)
{
    ServSocket* p_serv = (ServSocket*)args;
    if (!p_serv)
        return nullptr;
    std::cout << "Start Heart Handler!!!!!\n";
    while (true) {
        for (auto iter = p_serv->m_clnt_map.begin(); iter != p_serv->m_clnt_map.end();)
        {
            int& cur_count = iter->second.count;
            if (cur_count >= 5)
            {
                  std::cout << "Client [" <<iter->second.ip_addr << ": " <<iter->first <<" ], Heartbeat Failed\n";
                  close(iter->first);
                  p_serv->m_clnt_map.erase(iter++);
            }
            else if (cur_count >=0 && cur_count < 5)
            {
                cur_count++;
                ++iter;
            }
            else
            {
                std::cout << "Client [" <<iter->second.ip_addr << ": " <<iter->first <<" ], Count Error\n";
                close(iter->first);
                p_serv->m_clnt_map.erase(iter++);
            }
        }
        sleep(2);
    }
}

void* RecvHandler(void* args)
{
    pthread_args_t* pthread_args = (pthread_args_t*) args;
    if (!pthread_args)
        return nullptr;

    while(true)
    {
        int recv_size = 0;
        char buf[MAX_BUF_SIZE];
        memset(buf, 0, MAX_BUF_SIZE);
        recv_size = recv(pthread_args->new_fd, buf, MAX_BUF_SIZE, 0);

        if (recv_size < 0)
        {
            std::cout << "Recv Failed\n";
            exit(-1);
        }
        else if (0 == recv_size)
        {
            std::cout << "Recv Finished\n";
            close(pthread_args->new_fd);
            pthread_exit(NULL);
        }
        else
        {
            int8_t type = static_cast<int8_t>(buf[0]);
            uint32_t length = 0;
            memcpy(&length, buf + 1, sizeof(uint32_t));
            std::cout << "Type: " << +type << ", Lenght: " << length << std::endl;
            if (TYPE_TICK == type)
            {
                pthread_args->p_serv->ResetTickCount(pthread_args->new_fd);
                std::cout << "Recv Client Heart";
            }
            else if (TYPE_COMMON == type)
            {
                std::string recv_data(buf + 1 + sizeof(uint32_t),  length);
                std::cout <<"Recf Size:" << recv_size <<  ", Clinett Msg: " << recv_data << std::endl;
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
                    send(pthread_args->new_fd, send_data.c_str(), send_data.length(), 0);
                }
                else
                {
                    std::cout << "Not Correct operator\n";
                    std::string send_data("Not correct operator\n");
                    send(pthread_args->new_fd, send_data.c_str(), send_data.length(), 0);
                }
            }
        }
    }
}

void ServSocket::Run()
{
    pthread_t pd = -1;
    int ret = pthread_create(&pd, NULL, HeartHandler, (void*)this);
    if (ret !=0)
    {
        std::cout << "Pthread Create HeartHandler Failed!\n";
        exit(-1);
    }
    struct sockaddr_in clnt_addr;
    while (true) {
       std::cout << "Watting Client: \n";
       socklen_t clnt_len =sizeof(clnt_addr);
       int clnt_fd = accept(m_sockfd, (struct sockaddr*)&clnt_addr,
                            &clnt_len);
       if (clnt_fd < 0)
           continue;

       std::cout << "From Client: [" << inet_ntoa(clnt_addr.sin_addr) << "], fd: ["<< clnt_fd << "]"<< std::endl;
       pd = -1;
       pthread_args_t args;
       args.new_fd = clnt_fd;
       args.p_serv = this;
       ret = pthread_create(&pd, NULL, RecvHandler, (void*)&args);
       if (ret != 0)
       {
           std::cout << "Pthread Create RecvHandler Failed!\n";
           exit(-1);
       }
       clnt_info_t clnt_info;
       clnt_info.ip_addr = inet_ntoa(clnt_addr.sin_addr);
       m_clnt_map.insert(std::make_pair(clnt_fd, clnt_info));
    }
}

void ServSocket::ResetTickCount(int fd)
{
    auto iter = m_clnt_map.find(fd);
    if (iter != m_clnt_map.end())
    {
        iter->second.count = 0;
    }
}


ClntSocket::ClntSocket(int port, const std::string &ip)
    : Socket(port, ip)
{

}

void* SendHeart(void* args)
{
    std::cout << "The heartbeat sending thread started.\n";
    int fd = *((int*)args);
    int count = 0;
    while (true) {
        int8_t type = TYPE_TICK;
        uint32_t length = 0;
        ByteBuffer tmp_buff;
        tmp_buff << type << length;
        std::string send_data(tmp_buff.contents(), tmp_buff.size());
        send(fd, send_data.c_str(), send_data.length(), 0);
        sleep(2);
        ++count;
        if (count > 20)
        {
            std::cout << "HeartBeat Count Finished!\n";
            break;
        }
    }
}

void ClntSocket::Run()
{
    pthread_t id;
    int ret = pthread_create(&id, NULL, SendHeart, (void*)&m_sockfd);
    if(ret != 0)
    {
        std::cout << "Can not create thread!";
        exit(1);
    }

    int8_t type = TYPE_COMMON;
    std::string get_str("GET");
    uint32_t length = get_str.length();
    ByteBuffer tmp_buff;
    tmp_buff << type << length << get_str.c_str();
    std::string send_data(tmp_buff.contents(), tmp_buff.size());
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
