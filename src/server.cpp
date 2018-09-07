#include <iostream>
#include "socket.h"

using namespace std;

#define ASSERT(x) assert(x)
constexpr unsigned int MAX_BUF_SIZE = 128;
constexpr unsigned int BACKLOG = 10;
constexpr unsigned int PORT = 8888;
#define ANY_ADDRESS "0.0.0.0"

int main(int argc, char* argv[])
{
    ServSocket obj(PORT, ANY_ADDRESS, BACKLOG);
    int ret = obj.Init();
    if (ret != 0)
    {
        std::cout << "Errcode: " << ret << std::endl;
        return ret;
    }
    obj.Run();
    return 0;
}
