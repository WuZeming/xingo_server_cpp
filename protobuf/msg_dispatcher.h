#ifndef MSG_DISPATCHER_H_
#define MSG_DISPATCHER_H_
#include "muduo/net/Buffer.h"
#include "muduo/net/TcpConnection.h"
#include <google/protobuf/message.h>

//////////////////////////////以下内容摘自muduo，为了实现不同的msg回调函数的注册////////////////////////////
typedef std::shared_ptr<google::protobuf::Message> MessagePtr;
typedef std::function<void(const muduo::net::TcpConnectionPtr &,
                           const MessagePtr &message,
                           muduo::Timestamp)>
    ProtobufMessageCallback;

class Callback : muduo::noncopyable
{
public:
    virtual ~Callback() = default;
    virtual void onMessage(const muduo::net::TcpConnectionPtr &,
                           const MessagePtr &message,
                           muduo::Timestamp) const = 0;
};

template <typename T>
class CallbackT : public Callback
{
    static_assert(std::is_base_of<google::protobuf::Message, T>::value,
                  "T must be derived from gpb::Message.");

public:
    typedef std::function<void(const muduo::net::TcpConnectionPtr &,
                               const std::shared_ptr<T> &message,
                               muduo::Timestamp)>
        ProtobufMessageTCallback;

    CallbackT(const ProtobufMessageTCallback &callback)
        : callback_(callback)
    {
    }

    void onMessage(const muduo::net::TcpConnectionPtr &conn,
                   const MessagePtr &message,
                   muduo::Timestamp receiveTime) const override
    {
        std::shared_ptr<T> concrete = muduo::down_pointer_cast<T>(message);
        assert(concrete != NULL);
        callback_(conn, concrete, receiveTime);
    }

private:
    ProtobufMessageTCallback callback_;
};

////////////////////////////////////////////////////////////////////////////////////

class MsgDispatcher
{
public:
    explicit MsgDispatcher(ProtobufMessageCallback default_cb);
    ~MsgDispatcher();

    void onBufferMessage(const muduo::net::TcpConnectionPtr &conn,
                         muduo::net::Buffer *buff,
                         muduo::Timestamp reveiveTime);

    void send(const muduo::net::TcpConnectionPtr &conn,
              uint32_t msg_id,
              const google::protobuf::Message &message)
    {
        // FIXME: serialize to TcpConnection::outputBuffer()
        muduo::net::Buffer buf;
        fillEmptyBuffer(&buf, msg_id, message);
        conn->send(&buf);
    }

    template <typename T>
    void registMsgCallback(uint32_t msgid, const std::string &name, const typename CallbackT<T>::ProtobufMessageTCallback cbk)
    {

        std::shared_ptr<CallbackT<T>> pd(new CallbackT<T>(cbk));
        callbacks_[msgid] = pd;
        id2namemap_[msgid] = name;
    }
    static google::protobuf::Message *createMessage(const std::string &type_name);
    static void fillEmptyBuffer(muduo::net::Buffer *buf, uint32_t msg_id, const google::protobuf::Message &message);

private:
    typedef std::map<uint32_t, std::shared_ptr<Callback>> CallbackMap;

    CallbackMap callbacks_;
    std::unordered_map<uint32_t, std::string> id2namemap_;
    ProtobufMessageCallback defaultCallback_;
};

#endif