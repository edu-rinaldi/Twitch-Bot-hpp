cmake_minimum_required(VERSION 3.22.3 FATAL_ERROR)

project(twitch_bot VERSION 1.0.0)

# This library requires C++20
set(CMAKE_CXX_STANDARD 20)

add_subdirectory(single_include/)
add_subdirectory(example/)