#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <cassert>
#include <unistd.h>
#include "proto/user.pb.h"

using namespace std;

#define ASSERT(x) assert(x)
constexpr unsigned int MAX_BUF_SIZE = 128;
constexpr unsigned int BACKLOG = 10;
constexpr unsigned int PORT = 8888;


int main(int argc, char* argv[])
{
    int serv_fd = socket(PF_INET, SOCK_STREAM, 0);
    ASSERT(serv_fd != -1);

    int reuseaddr_on = 1;
    int ret = setsockopt(serv_fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr_on, sizeof(reuseaddr_on));
    ASSERT(ret != -1);

    struct sockaddr_in serv_addr, clnt_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    ret = bind(serv_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    ASSERT(ret != -1);

    ret = listen(serv_fd, BACKLOG);
    ASSERT(-1 != ret);

    while(true)
    {
        std::cout << "Waitting For Client: \n";
        socklen_t clnt_length = sizeof(clnt_addr);
        int clnt_fd = accept(serv_fd, (struct sockaddr*)&clnt_addr, &clnt_length);
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


    close(serv_fd);
    return 0;
}
