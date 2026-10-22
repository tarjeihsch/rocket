add_executable(ColorTest Color.cpp)
target_link_libraries(ColorTest PUBLIC Core)

add_executable(DelegateTest Delegate.cpp)
target_link_libraries(DelegateTest PUBLIC Core)
