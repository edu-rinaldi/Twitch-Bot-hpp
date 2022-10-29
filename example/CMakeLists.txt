project(twitch_bot_example VERSION 1.0.0 LANGUAGES CXX)

# Example's executable
add_executable(${PROJECT_NAME} sample.cpp)

# link twitch_bot in target (in this case is the twitch_bot_example executable)
target_link_libraries(${PROJECT_NAME} PUBLIC twb)