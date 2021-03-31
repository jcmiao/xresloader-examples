#include "pool.h"

google::protobuf::Arena* __arena__ = nullptr;
google::protobuf::Arena* InitArena(char* initial_block, size_t initial_block_size)
{
    __arena__ = new google::protobuf::Arena(initial_block, initial_block_size);
    return __arena__;
}

google::protobuf::Arena* GetArena()
{
    if (!__arena__)
    {
        const int PoolSize = 1024*1024*10;
        char* mem = new char[PoolSize];
        __arena__ = new google::protobuf::Arena(mem, PoolSize);
    }
    return __arena__;
}

char* __config_manager__ = nullptr;
char* InitConfigManagerPtr(size_t objsize)
{
    __config_manager__ = new char[objsize];
    return __config_manager__;
}
char* GetConfigManagerPtr()
{
    return __config_manager__;
}