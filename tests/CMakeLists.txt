# ----------------------------------------------------------------------
# test_demo
# ----------------------------------------------------------------------
# 一个测试目标本质上也是一个普通 C++ 可执行程序。
# 区别只是它链接了 GTest::gtest_main，并且被 CTest 注册为测试。
add_executable(test_demo
        test_demo.cpp
)

target_link_libraries(test_demo
        PRIVATE
        get_input
        error_code
        lib_log
        GTest::gtest_main
)

# GoogleTest 模块提供 gtest_discover_tests。
# 它会让 CTest 自动发现 TEST(...) 宏定义出来的测试用例。
include(GoogleTest)

gtest_discover_tests(
        test_demo

        # 对 RK3588 场景更友好：
        # - 如果你是在板端原生编译，POST_BUILD/PRE_TEST 都能工作；
        # - 如果以后在 PC 上交叉编译 ARM 程序，POST_BUILD 可能会尝试在 x86 PC 上运行 ARM 测试程序；
        # - PRE_TEST 会尽量把测试发现推迟到 ctest 执行前，更适合“构建”和“运行测试”分离的嵌入式流程。
        DISCOVERY_MODE PRE_TEST
)