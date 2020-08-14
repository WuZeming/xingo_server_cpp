#include "game_core/World.h"


enum
{
    kBroadCast = 200,
    kSyncPlayers = 202,
    kLoseConn = 201
};

World::World()
    : nextId_(1)
{
}

World::~World()
{
}

void World::addPlayer(TcpConnectionPtr pconn, MsgDispatcher *dispatcher)
{
    // world是单例，多线程互斥修改
    auto newplayer = Player::NewPlayer(nextId_, pconn, dispatcher);
    {
        muduo::MutexLockGuard mutexlock(mutex_);
        players_[nextId_++] = newplayer;
    }
    pconn->setPlayerId(newplayer->getPid());
    
    newplayer->syncId();
    newplayer->syncSurround();
}
void World::removePlayer(Pid pid)
{
    {
        muduo::MutexLockGuard mutexlock(mutex_);
        players_.erase(pid);
    }
}

SpPlayer World::getPlayer(Pid pid)
{
    {
        muduo::MutexLockGuard mutexlock(mutex_);
        auto res = players_.find(pid);
        if(res!=players_.end())
        {
            return res->second;
        }
    }
}

std::vector<uint32_t>  World::getAllPlayersId()
{
    std::vector<uint32_t> res;
    {
        for(auto& item : players_)
        {
            res.emplace_back(item.first);
        }
    }
    return res;
}

void World::move(Player* player)
{
    //std::cout<<"move "<<std::endl;

    pb::BroadCast msg;
    msg.set_pid(player->getPid());
    msg.set_tp(4);
    pb::Position* pos = msg.mutable_p();
    pos->set_x(player->getPosition().X);
    pos->set_y(player->getPosition().Y);
    pos->set_z(player->getPosition().Z);
    pos->set_v(player->getPosition().R);
    this->broadcast(kBroadCast,msg);
}
void World::broadcast(int msgId,const google::protobuf::Message& data)
{
    {
        muduo::MutexLockGuard mutexlock(mutex_);
        for(auto& item: players_)
        {
            item.second->sendmsg(msgId,data);
        }
    }
}