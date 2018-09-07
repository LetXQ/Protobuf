#include <iostream>
#include "socket.h"

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
    ClntSocket obj(port, localhost);
    int ret = obj.Init();
    if (ret != 0)
    {
        std::cout << "Errcode: " << ret << std::endl;
        return ret;
    }
    obj.Run();

    return 0;
}
