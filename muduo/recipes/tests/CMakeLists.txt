add_executable(reactor_test_01 ReactorTest_01.cpp)
target_link_libraries(reactor_test_01 muduo_recipes)

add_executable(reactor_test_02 ReactorTest_02.cpp)
target_link_libraries(reactor_test_02 muduo_recipes)

add_executable(reactor_test_03 ReactorTest_03.cpp)
target_link_libraries(reactor_test_03 muduo_recipes)

add_executable(reactor_test_04 ReactorTest_04.cpp)
target_link_libraries(reactor_test_04 muduo_recipes)

add_executable(reactor_test_05 ReactorTest_05.cpp)
target_link_libraries(reactor_test_05 muduo_recipes)

add_executable(reactor_test_06 ReactorTest_06.cpp)
target_link_libraries(reactor_test_06 muduo_recipes)

if(BOOSTTEST_LIBRARY)
add_executable(inetaddress_unittest InetAddress_Unittest.cpp)
target_link_libraries(inetaddress_unittest muduo_recipes boost_unit_test_framework)
endif()

#add_executable(reactor_test_07 ReactorTest_07.cpp)
#target_link_libraries(reactor_test_07 muduo_recipes)
#
#add_executable(reactor_test_08 ReactorTest_08.cpp)
#target_link_libraries(reactor_test_08 muduo_recipes)
#
#add_executable(reactor_test_09 ReactorTest_09.cpp)
#target_link_libraries(reactor_test_09 muduo_recipes)
#
#add_executable(reactor_test_10 ReactorTest_10.cpp)
#target_link_libraries(reactor_test_10 muduo_recipes)

add_executable(reactor_test_11 ReactorTest_11.cpp)
target_link_libraries(reactor_test_11 muduo_recipes)

add_executable(reactor_test_12 ReactorTest_12.cpp)
target_link_libraries(reactor_test_12 muduo_recipes)