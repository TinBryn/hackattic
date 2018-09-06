cmake_minimum_required(VERSION 3.12)
project(hackattic)

set(CMAKE_CXX_STANDARD 14)

add_executable(hackattic main.cpp base64.hpp base64_test.cpp)