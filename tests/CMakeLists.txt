# 单元测试使用独立可执行文件，避免测试入口影响 demo 主程序。
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

include(GoogleTest)

gtest_discover_tests(
        test_demo

        # 避免在构建阶段运行测试程序，更适合远端板端和交叉编译场景。
        DISCOVERY_MODE PRE_TEST
)
