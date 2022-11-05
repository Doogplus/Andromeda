set(recipes_SRCS
        Acceptor.cpp
        Channel.cpp
        EventLoop.cpp
        EventLoopThread.cpp
        EventLoopThreadPool.cpp
        InetAddress.cpp
        Poller.cpp
        poller/DefaultPoller.cpp
        poller/PollPoller.cpp
        poller/EPollPoller.cpp
        Socket.cpp
        SocketsOps.cpp
        TcpConnection.cpp
        TcpServer.cpp
        Timer.cpp
        TimerQueue.cpp)

add_library(muduo_recipes ${recipes_SRCS})
target_link_libraries(muduo_recipes muduo_base)

install(TARGETS muduo_recipes DESTINATION lib)
set(HEADERS
        Acceptor.h
        Channel.h
        EventLoop.h
        EventLoopThread.h
        EventLoopThreadPool.h
        Endian.h
        Poller.h
        InetAddress.h
        TcpConnection.h
        TcpServer.h
        TimerId.h)
install(FILES ${HEADERS} DESTINATION include/muduo/recipes)

add_subdirectory(tests)