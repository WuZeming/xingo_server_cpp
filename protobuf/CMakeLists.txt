add_library(msg msg.pb.cc)
target_link_libraries(msg protobuf)

add_library(codec msg_dispatcher.cpp)

target_link_libraries(codec msg muduo_net)