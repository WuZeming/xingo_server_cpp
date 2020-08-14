#ifndef PLAYER_H_
#define PLAYER_H_
#include "stdint.h"
#include <memory>
#include "protobuf/msg_dispatcher.h"
#include "protobuf/msg.pb.h"
struct Position
{
    float X;
    float Y;
    float Z;
    float R;
};
class TcpConnection;
class Player;
using muduo::net::TcpConnectionPtr;
using SpPlayer = std::shared_ptr<Player>;
using Pid = uint32_t;

class Player
{
public:
    Player(Pid pid, TcpConnectionPtr pconn, MsgDispatcher *dsp,
           float x,
           float y,
           float z,
           float r = 0); // 默认方向为0
    ~Player();

    static SpPlayer NewPlayer(Pid pid, TcpConnectionPtr pconn, MsgDispatcher *dsp);
    Pid getPid() {return pid_;}
    const Position& getPosition()const {return pos_;}
    void talk(std::string content);
    void sendmsg(uint32_t msgId,const google::protobuf::Message& msg);
    void updatePos(const Position& pos);
    void syncSurround();
    void loseConn();
    void syncId();// 向客户端发送自己的id

private:
    /* data */
    Position pos_;              // 玩家位置
    int32_t pid_;               // 玩家ID
    TcpConnectionPtr pConn_;    // 玩家所属连接
    MsgDispatcher *dispatcher_; //
};

#endif