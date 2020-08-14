#include "muduo/net/TcpServer.h"
#include "muduo/net/EventLoop.h"
#include "muduo/base/Logging.h"
#include "protobuf/msg.pb.h"
#include "protobuf/msg_dispatcher.h"
#include "game_core/World.h"
using namespace muduo;
using namespace muduo::net;
#include <sstream>
enum
{
    kPosition = 3,
    kTalk = 2,
};

class GameServer
{
public:
    GameServer(EventLoop *loop, const InetAddress &listenAddr)
        : server_(loop, listenAddr, "GameServer"),
          dispatcher_(std::bind(&GameServer::unKnownMessage, this, _1, _2, _3))
    {
        server_.setConnectionCallback(std::bind(&GameServer::onConnection, this, _1));
        server_.setMessageCallback(std::bind(&MsgDispatcher::onBufferMessage, &dispatcher_, _1, _2, _3));
        dispatcher_.registMsgCallback<pb::Position>(kPosition, "pb.Position", std::bind(&GameServer::onPosition, this, _1, _2, _3));
        dispatcher_.registMsgCallback<pb::Talk>(kTalk, "pb.Talk", std::bind(&GameServer::onTalk, this, _1, _2, _3));
        server_.setThreadNum(5);
    }
    ~GameServer()
    {
    }
    void start()
    {
        server_.start();
        LOG_INFO << "server started\n";
    }

private:
    TcpServer server_;
    MsgDispatcher dispatcher_;

    void onConnection(const TcpConnectionPtr &conn) // 发生连接或者断开会调用该函数
    {
        if (conn->connected()) // 玩家上线
        {
            World::GetWorld()->addPlayer(conn, &dispatcher_);
            LOG_INFO << conn->getPlayerId() << ": connected/n";
        }
        else // 玩家下线
        {
            auto player = World::GetWorld()->getPlayer(conn->getPlayerId());
            World::GetWorld()->removePlayer(player->getPid()); // 从世界中移除玩家
            player->loseConn();                                // 玩家从地图消失
            LOG_INFO << conn->getPlayerId() << ": closed/n";
        }
    }

    void unKnownMessage(const muduo::net::TcpConnectionPtr &,
                        const MessagePtr &message,
                        muduo::Timestamp)
    {
        LOG_INFO << "UnKnown Message\n";
    }

    void onPosition(const muduo::net::TcpConnectionPtr &conn,
                    const std::shared_ptr<pb::Position> &message,
                    muduo::Timestamp time)
    {
        auto player = World::GetWorld()->getPlayer(conn->getPlayerId());
        Position pos = {message->x(), message->y(), message->z(), message->v()};
        player->updatePos(pos);
        LOG_INFO << conn->getPlayerId() << ": moved/n"<<time.toString();
    }

    void onTalk(const muduo::net::TcpConnectionPtr &conn,
                const std::shared_ptr<pb::Talk> &message,
                muduo::Timestamp)
    {
        auto player = World::GetWorld()->getPlayer(conn->getPlayerId());
        player->talk(message->content());
        LOG_INFO << conn->getPlayerId() << ": talk: " << message->content() << "/n";
    }
};

int main()
{
    EventLoop mainloop;
    InetAddress addr(static_cast<uint16_t>(8999));
    GameServer server(&mainloop, addr);
    server.start();
    mainloop.loop();
}