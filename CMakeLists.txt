cmake_minimum_required(VERSION 3.18)

project(gtest_demo LANGUAGES CXX)

# ----------------------------------------------------------------------
# C++ 标准
# ----------------------------------------------------------------------
# GoogleTest v1.17.0 已经要求至少 C++17。
# 当前工程本身也适合用 C++17，因此这里统一固定为 C++17。
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ----------------------------------------------------------------------
# 可执行文件输出目录
# ----------------------------------------------------------------------
# 所有 add_executable(...) 生成的程序都会放到：
#   build/debug/bin/
#   build/release/bin/
#
# 这样主程序 gtest_demo 和测试程序 get_input_test 都集中在 bin 目录下。
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# ----------------------------------------------------------------------
# 工程内部库
# ----------------------------------------------------------------------
# lib/ 下的 log、error_code、get_input 会先被加入工程。
# 后面的主程序和测试程序都可以链接这些库。
add_subdirectory(lib)

# ----------------------------------------------------------------------
# 主程序
# ----------------------------------------------------------------------
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

# ----------------------------------------------------------------------
# 单元测试开关
# ----------------------------------------------------------------------
# include(CTest) 会提供一个标准 CMake 选项：
#   BUILD_TESTING=ON/OFF
#
# 默认通常是 ON。
# 如果你只想构建主程序，不想下载/编译 gtest，可以这样：
#   cmake -S . -B build/debug -DBUILD_TESTING=OFF
include(CTest)

if(BUILD_TESTING)
    # FetchContent 会在 CMake 配置阶段下载并加入第三方依赖。
    # 对初学阶段来说，它比手动安装 libgtest-dev 更稳定：
    #   1. 不依赖系统包版本；
    #   2. 换一台机器也能按同一个版本构建；
    #   3. CMake 代码更自包含。
    include(FetchContent)

    # 不把 GoogleTest 安装进本项目的 install 结果里。
    # 对你的工程来说，gtest 只是“测试时依赖”，不是运行 gtest_demo 所需的库。
    set(INSTALL_GTEST OFF CACHE BOOL "Do not install GoogleTest with this project" FORCE)

    FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG v1.17.0
    )

    FetchContent_MakeAvailable(googletest)

    # 测试代码单独放在 tests/，避免污染主程序源码。
    add_subdirectory(tests)
endif()