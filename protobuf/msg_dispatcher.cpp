#include "protobuf/msg_dispatcher.h"
#include "protobuf/google-inl.h"

MsgDispatcher::MsgDispatcher(ProtobufMessageCallback default_cb)
    : defaultCallback_(default_cb)
{
}

MsgDispatcher::~MsgDispatcher()
{
}

void MsgDispatcher::onBufferMessage(const muduo::net::TcpConnectionPtr &conn,
                                    muduo::net::Buffer *buff,
                                    muduo::Timestamp receiveTime)
{
    //debug
    //std::cout<<"on message!!! readable buff"<<buff->readableBytes()<<std::endl;

    while (buff->readableBytes() >= 2 * sizeof(uint32_t)) // buff中的字节数大于消息头
    {
        const uint32_t len = buff->peekInt32_little_endian();
        if (buff->readableBytes() >= static_cast<size_t>(len + 2 * sizeof(uint32_t)))
        {
            buff->retrieveInt32();
            const uint32_t msg_id = buff->peekInt32_little_endian();
            buff->retrieveInt32();
            MessagePtr message(createMessage(id2namemap_[msg_id]));
            message->ParseFromArray(buff->peek(), len);
            CallbackMap::const_iterator it = callbacks_.find(msg_id);
            if (it != callbacks_.end())
            {
                it->second->onMessage(conn, message, receiveTime);
                buff->retrieve(len);
            }
            else
            {
                defaultCallback_(conn, message, receiveTime);
            }
        }
        else
        {
            break;
        }
    }
}

google::protobuf::Message *MsgDispatcher::createMessage(const std::string &typeName)
{
    google::protobuf::Message *message = NULL;
    const google::protobuf::Descriptor *descriptor =
        google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(typeName);
    if (descriptor)
    {
        const google::protobuf::Message *prototype =
            google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
        if (prototype)
        {
            message = prototype->New();
        }
    }
    return message;
}

void MsgDispatcher::fillEmptyBuffer(muduo::net::Buffer *buf, uint32_t msg_id, const google::protobuf::Message &message)
{

    assert(buf->readableBytes() == 0);
    int byte_size = google::protobuf::internal::ToIntSize(message.ByteSizeLong());
    ///std::cout<<byte_size<<std::endl;
    
    buf->appendInt32_little_endian(byte_size);// 消息头长度
    buf->appendInt32_little_endian(msg_id);// 消息类型

    // code copied from MessageLite::SerializeToArray() and MessageLite::SerializePartialToArray().
    GOOGLE_DCHECK(message.IsInitialized()) << InitializationErrorMessage("serialize", message);

    buf->ensureWritableBytes(byte_size);

    uint8_t *start = reinterpret_cast<uint8_t *>(buf->beginWrite());
    uint8_t *end = message.SerializeWithCachedSizesToArray(start);
    if (end - start != byte_size)
    {
        ByteSizeConsistencyError(byte_size, google::protobuf::internal::ToIntSize(message.ByteSizeLong()), static_cast<int>(end - start));
    }
    buf->hasWritten(byte_size);

}