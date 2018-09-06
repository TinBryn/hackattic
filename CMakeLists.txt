cmake_minimum_required(VERSION 3.12)
project(hackattic)

set(CMAKE_CXX_STANDARD 14)

include_directories(common/include)

add_executable(hackattic
        common/include/base64.hpp
        common/test/base64_test.cpp
        main.cpp
        README.md)
