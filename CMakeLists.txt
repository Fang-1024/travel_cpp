cmake_minimum_required(VERSION 3.28)

project(gtest_demo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(lib)

add_executable(gtest_demo
        main.cpp
)
target_link_libraries(gtest_demo lib_log)
target_link_libraries(gtest_demo error_code)
target_link_libraries(gtest_demo get_input)