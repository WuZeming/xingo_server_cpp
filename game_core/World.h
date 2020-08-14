#ifndef WORLD_H_
#define WORLD_H_
#include <unordered_map>
#include <memory>
#include "game_core/Player.h"
#include "muduo/base/Mutex.h"
#include "protobuf/msg_dispatcher.h"
class World
{

public:
    World();
    ~World();
    static World *GetWorld() // singlenton
    {
        static World world;
        return &world;
    }
    void addPlayer(TcpConnectionPtr pconn, MsgDispatcher *dispacher);
    void removePlayer(Pid pid);
    SpPlayer getPlayer(Pid pid);
    void move(Player* player);
    void broadcast(int msgId, const google::protobuf::Message &data);
    std::vector<uint32_t> getAllPlayersId();

private:
    /* data */
    muduo::MutexLock mutex_;
    uint32_t nextId_;
    std::unordered_map<uint32_t, SpPlayer> players_;
};

#endif