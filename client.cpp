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
const char* localhost = "127.0.0.1";

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " port\n";
        return 1;
    }
    int port = atoi(argv[1]);
    if (port < 2000)
    {
        std::cout << "Port not correct!\n";
        return 2;
    }
    int serv_fd = socket(PF_INET, SOCK_STREAM, 0);
    ASSERT(serv_fd != -1);

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    inet_pton(AF_INET, localhost, &serv_addr.sin_addr);

    int ret = connect(serv_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    ASSERT(ret != -1);

    std::string send_data("GET");
    int send_bytes = send(serv_fd, send_data.c_str(), send_data.length(), 0);
    if (send_bytes != send_data.length())
    {
        std::cout << "Send Error\n";
        return 3;
    }
    char buf[MAX_BUF_SIZE];
    memset(buf, 0, MAX_BUF_SIZE);

    int num_bytes = recv(serv_fd, buf, MAX_BUF_SIZE, 0);
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

    close(serv_fd);
    return 0;
}
