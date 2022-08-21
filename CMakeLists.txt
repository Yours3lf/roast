cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(roast LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(roast Server.cpp)

