cmake_minimum_required(VERSION 3.20)
project(homemade_vector)

set(CMAKE_CXX_STANDARD 20)

add_executable(homemade_vector main.cpp
        Vector.cpp
        Vector.hpp
)
