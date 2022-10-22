project(twitch_bot_example VERSION 1.0.0 LANGUAGES CXX)

add_executable(${PROJECT_NAME} sample.cpp)

# include header
target_include_directories(${PROJECT_NAME} PUBLIC ../)