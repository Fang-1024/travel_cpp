#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "error_code.h"
#include "get_input.h"

namespace {

    // ------------------------------------------------------------------
    // ArgvCase：给 get_input 构造一个“像 main(argc, argv) 一样”的测试输入。
    // ------------------------------------------------------------------
    //
    // main 函数里的 argv 类型是 char* argv[]，但在测试里我们更容易写：
    //
    //   {"app", "--name", "alice"}
    //
    // 所以这里做一层小封装：
    //   1. storage 保存真正的字符串内容；
    //   2. argv 保存每个字符串内部 buffer 的 char* 指针；
    //   3. 最后额外补一个 nullptr，模拟 C/C++ 对 argv[argc] 的惯例。
    //
    // 注意：
    //   get_input 当前只会移动 argv 里的指针，不会修改字符串内容本身，
    //   因此使用 std::string::data() 是可以的。
    struct ArgvCase {
        std::vector<std::string> storage;
        std::vector<char *> argv;
        int argc = 0;

        explicit ArgvCase(std::vector<std::string> args)
                : storage(std::move(args)),
                  argc(static_cast<int>(storage.size())) {
            argv.reserve(storage.size() + 1);

            for (std::string &arg: storage) {
                argv.push_back(arg.data());
            }

            // 模拟真实 argv 结尾。
            // get_input 里 erase_argv_range(...) 也会主动维护 argv[argc] = nullptr。
            argv.push_back(nullptr);
        }
    };

} // namespace

TEST(GetInputTest, ReadLongOptionEqualForm) {
    ArgvCase args({
                          "app",
                          "--name=alice",
                          "--other",
                  });

    std::string output;
    const auto code = input_args::get_input(args.argc, args.argv.data(), "--name", output);

    EXPECT_EQ(error_code::ErrorCode::Ok, code);
    EXPECT_EQ("alice", output);

    // "--name=alice" 被解析后应该从 argv 中移除。
    EXPECT_EQ(2, args.argc);
    EXPECT_STREQ("app", args.argv[0]);
    EXPECT_STREQ("--other", args.argv[1]);
    EXPECT_EQ(nullptr, args.argv[2]);
}

TEST(GetInputTest, ReadLongOptionSeparatedForm) {
    ArgvCase args({
                          "app",
                          "--name",
                          "alice",
                          "--other",
                  });

    std::string output;
    const auto code = input_args::get_input(args.argc, args.argv.data(), "--name", output);

    EXPECT_EQ(error_code::ErrorCode::Ok, code);
    EXPECT_EQ("alice", output);

    // "--name" 和 "alice" 两个 argv 元素都会被移除。
    EXPECT_EQ(2, args.argc);
    EXPECT_STREQ("app", args.argv[0]);
    EXPECT_STREQ("--other", args.argv[1]);
    EXPECT_EQ(nullptr, args.argv[2]);
}

TEST(GetInputTest, ReadShortOptionSeparatedForm) {
    ArgvCase args({
                          "app",
                          "-n",
                          "alice",
                  });

    std::string output;
    const auto code = input_args::get_input(args.argc, args.argv.data(), "-n", output);

    EXPECT_EQ(error_code::ErrorCode::Ok, code);
    EXPECT_EQ("alice", output);

    EXPECT_EQ(1, args.argc);
    EXPECT_STREQ("app", args.argv[0]);
    EXPECT_EQ(nullptr, args.argv[1]);
}

TEST(GetInputTest, ReturnUnknownWhenOptionDoesNotExist) {
    ArgvCase args({
                          "app",
                          "--other",
                          "value",
                  });

    std::string output = "unchanged";
    const auto code = input_args::get_input(args.argc, args.argv.data(), "--name", output);

    EXPECT_EQ(error_code::ErrorCode::Unknown, code);

    // 没找到参数时，当前实现不会修改 output。
    EXPECT_EQ("unchanged", output);

    // 没找到参数时，也不应该修改 argc/argv。
    EXPECT_EQ(3, args.argc);
    EXPECT_STREQ("app", args.argv[0]);
    EXPECT_STREQ("--other", args.argv[1]);
    EXPECT_STREQ("value", args.argv[2]);
}

TEST(GetInputTest, ReturnUnknownWhenOptionHasNoValue) {
    ArgvCase args({
                          "app",
                          "--name",
                  });

    std::string output;
    const auto code = input_args::get_input(args.argc, args.argv.data(), "--name", output);

    EXPECT_EQ(error_code::ErrorCode::Unknown, code);
}

TEST(GetInputTest, ReturnParseFailedWhenNextArgumentLooksLikeOption) {
    ArgvCase args({
                          "app",
                          "--name",
                          "--other",
                  });

    std::string output;
    const auto code = input_args::get_input(args.argc, args.argv.data(), "--name", output);

    // 当前实现认为 "--name --other" 中的 "--other" 更像另一个选项，
    // 不应该被误当成 "--name" 的 value。
    EXPECT_EQ(error_code::ErrorCode::ParseFailed, code);
}

TEST(GetInputTest, SupportMultipleCandidateOptionNames) {
    ArgvCase args({
                          "app",
                          "-n=bob",
                  });

    std::string output;
    const auto code = input_args::get_input(
            args.argc,
            args.argv.data(),
            std::vector<std::string>{"--name", "-n"},
            output
    );

    EXPECT_EQ(error_code::ErrorCode::Ok, code);
    EXPECT_EQ("bob", output);
}