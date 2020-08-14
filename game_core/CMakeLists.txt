set(src
    Player.cpp
    World.cpp)
add_library(game_core ${src})
target_link_libraries(game_core codec muduo_net)
