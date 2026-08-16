cmake_minimum_required(VERSION 3.18)

project(gtest_demo LANGUAGES CXX)

# GoogleTest 1.17 和当前示例代码都使用 C++17。
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 统一可执行文件输出位置，便于 CLion、脚本和远端部署按固定路径查找。
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# 所有对外头文件都在 include/travel_cpp/ 下。
# 这里向各个库目标公开 include/ 这个“包含根目录”，因此调用者写的是
#   #include <travel_cpp/log.h>
# 而不是依赖本机相对路径的 "../include/..."。
set(TRAVEL_CPP_PUBLIC_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")

# 目录已经按 include/ 和 src/ 分层，库仍按职责拆成独立 CMake 目标。
# “物理目录统一”不等于“所有代码合并成一个库”。
add_library(lib_log
        src/log.cpp
        include/travel_cpp/log.h
)
target_include_directories(lib_log
        PUBLIC
        "${TRAVEL_CPP_PUBLIC_INCLUDE_DIR}"
)

add_library(error_code
        src/error_code.cpp
        include/travel_cpp/error_code.h
)
target_include_directories(error_code
        PUBLIC
        "${TRAVEL_CPP_PUBLIC_INCLUDE_DIR}"
)

add_library(get_input
        src/get_input.cpp
        include/travel_cpp/get_input.h
)
target_include_directories(get_input
        PUBLIC
        "${TRAVEL_CPP_PUBLIC_INCLUDE_DIR}"
)

# get_input.h 的公开 API 使用了 error_code::ErrorCode，所以用 PUBLIC 表示这是
# 消费者也需要看到的传递依赖。
target_link_libraries(get_input
        PUBLIC
        error_code
)

add_executable(gtest_demo
        src/main.cpp
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
