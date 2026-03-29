add_executable(CoreTest 
    Color.cpp
)

target_link_libraries(CoreTest 
    PRIVATE 
        Core
        Platform
)

add_test(NAME CoreTest COMMAND CoreTest)