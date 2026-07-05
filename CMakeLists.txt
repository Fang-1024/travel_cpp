cmake_minimum_required(VERSION 3.18)

project(gtest_demo LANGUAGES CXX)

# GoogleTest 1.17 和当前示例代码都使用 C++17。
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 统一可执行文件输出位置，便于 CLion、脚本和远端部署按固定路径查找。
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# 先加入内部库，主程序和测试目标都会链接这些库。
add_subdirectory(lib)

add_executable(gtest_demo
        main.cpp
)

target_link_libraries(gtest_demo
        PRIVATE
        lib_log
        error_code
        get_input
)

install(TARGETS gtest_demo
        RUNTIME DESTINATION bin
)

# CTest 提供 BUILD_TESTING 开关；远端只运行 demo 时可以设为 OFF。
include(CTest)

if(BUILD_TESTING)
    include(FetchContent)

    # gtest 只用于测试目标，不应出现在 demo 的安装结果中。
    set(INSTALL_GTEST OFF CACHE BOOL "Do not install GoogleTest with this project" FORCE)

    FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG v1.17.0
    )

    FetchContent_MakeAvailable(googletest)

    # 测试目标独立放在 tests/，避免和主程序入口混在一起。
    add_subdirectory(tests)
endif()
