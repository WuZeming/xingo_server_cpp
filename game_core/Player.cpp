#include "game_core/Player.h"
#include "game_core/World.h"
enum
{
    kBroadCast = 200,
    kSyncPlayers = 202,
    kLoseConn = 201,
    kSyncId = 1
};

Player::Player(Pid pid,
               TcpConnectionPtr pconn,
               MsgDispatcher *dsp,
               float x,
               float y,
               float z,
               float r)
    : pid_(pid),
      pConn_(pconn),
      dispatcher_(dsp),
      pos_({x, y, z, r}) //
{
}

Player::~Player()
{
}

SpPlayer Player::NewPlayer(Pid pid, TcpConnectionPtr pconn, MsgDispatcher *dsp)
{
    return std::make_shared<Player>(pid,
                                    pconn,
                                    dsp,
                                    static_cast<float>(rand() % 10 + 160),
                                    static_cast<float>(0),
                                    static_cast<float>(rand() % 17 + 134),
                                    static_cast<float>(0)); // 初始化玩家出生位置
}

void Player::talk(std::string content)
{
    pb::BroadCast data;
    data.set_pid(getPid());
    data.set_tp(1); //
    data.set_content(content.c_str());
    World::GetWorld()->broadcast(kBroadCast, data);
}

void Player::sendmsg(uint32_t msgId, const google::protobuf::Message &msg)
{
    dispatcher_->send(pConn_, msgId, msg);
    //std::cout<<getPid()<<" : send message"<<std::endl;
}

void Player::syncSurround()
{
    // 获取每个玩家的位置并发送给自己,   
    auto allPlayersid = World::GetWorld()->getAllPlayersId();
    pb::SyncPlayers data;
    for (auto i : allPlayersid)
    {
        auto item = World::GetWorld()->getPlayer(i);
        pb::Player *p = data.add_ps();
        p->set_pid(i);
        pb::Position* pos = p->mutable_p();
        pos->set_x(item->getPosition().X);
        pos->set_y(item->getPosition().Y);
        pos->set_z(item->getPosition().Z);
        pos->set_v(item->getPosition().R);

        //std::cout<<item->getPosition().X<<" "<<item->getPosition().Y<<" "<<item->getPosition().Z <<std::endl;
    }
    this->sendmsg(kSyncPlayers, data);
    //将自己的位置告知其他玩家、
    pb::BroadCast msg;
    msg.set_pid(getPid());
    msg.set_tp(2);
    pb::Position* pos = msg.mutable_p();
    pos->set_x(getPosition().X);
    pos->set_y(getPosition().Y);
    pos->set_z(getPosition().Z);
    pos->set_v(getPosition().R);
    World::GetWorld()->broadcast(kBroadCast, msg);
}

void Player::updatePos(const Position &pos)
{
    this->pos_ = pos;
    World::GetWorld()->move(this);
}

void Player::loseConn()
{
    pb::SyncPid msg;
    msg.set_pid(getPid());
    World::GetWorld()->broadcast(kLoseConn, msg);
}

void Player::syncId()
{
    pb::SyncPid msg;
    msg.set_pid(this->getPid());
    this->sendmsg(kSyncId,msg);
}