add_executable(CoreTest Main.cpp)

target_link_libraries(CoreTest PRIVATE Core)

add_test(NAME CoreTest COMMAND CoreTest)