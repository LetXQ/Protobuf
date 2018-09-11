#ifndef DATA_DEF_H
#define DATA_DEF_H
#include <cstdint>

enum cmd_type_t
{
    TYPE_NULL = -1,
    TYPE_TICK = 1,
    TYPE_COMMON = 2,
};

struct PacketHead
{
    int8_t type = TYPE_NULL;
    uint32_t length = 0;
};

#endif // CMD_MSG_H
