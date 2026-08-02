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
    # 使用目标机上已经安装的 GoogleTest，配置和构建过程不再访问网络。
    # 如 GoogleTest 安装在非标准位置，可在配置时传入：
    #   -DGTest_ROOT=/path/to/gtest
    find_package(GTest REQUIRED)

    # 新版 GTest 配置包使用 GTest::gtest_main，较旧的 CMake FindGTest
    # 模块可能只提供 GTest::Main；为测试目录选择实际存在的目标。
    if(TARGET GTest::gtest_main)
        set(TRAVEL_GTEST_MAIN_TARGET GTest::gtest_main)
    elseif(TARGET GTest::Main)
        set(TRAVEL_GTEST_MAIN_TARGET GTest::Main)
    else()
        message(FATAL_ERROR "GoogleTest was found, but no gtest_main target is available")
    endif()

    message(STATUS "Using system GoogleTest target: ${TRAVEL_GTEST_MAIN_TARGET}")

    # 测试目标独立放在 tests/，避免和主程序入口混在一起。
    add_subdirectory(tests)
endif()
