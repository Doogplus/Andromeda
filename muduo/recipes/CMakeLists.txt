set(recipes_SRCS
        EventLoop.cpp
        EventLoopThread.cpp
        Channel.cpp
        Poller.cpp
        poller/DefaultPoller.cpp
        poller/PollPoller.cpp
        poller/EPollPoller.cpp
        Timer.cpp
        TimerQueue.cpp)

add_library(muduo_recipes ${recipes_SRCS})
target_link_libraries(muduo_recipes muduo_base)

install(TARGETS muduo_recipes DESTINATION lib)
set(HEADERS
        EventLoop.h
        EventLoopThread.h
        Channel.h
        Poller.h
        TimerId.h)
install(FILES ${HEADERS} DESTINATION include/muduo/recipes)

add_subdirectory(tests)