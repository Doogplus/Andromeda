set(recipes_SRCS
        EventLoop.cpp)

add_library(muduo_recipes ${recipes_SRCS})
target_link_libraries(muduo_recipes muduo_base)

install(TARGETS muduo_recipes DESTINATION lib)
set(HEADERS
        EventLoop.h)
install(FILES ${HEADERS} DESTINATION include/muduo/recipes)

add_subdirectory(tests)