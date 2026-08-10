#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "error_code.h"
#include "get_input.h"
#include "log.h"

namespace
{
    // 为 get_input 构造接近 main(argc, argv) 的测试输入。
    // get_input 只移动 argv 指针，不修改字符串内容，因此可以直接引用 storage 的
    // buffer。
    struct ArgvCase
    {
        std::vector<std::string> storage;
        std::vector<char*> argv;
        int argc = 0;

        explicit ArgvCase(std::vector<std::string> args)
            : storage(std::move(args)), argc(static_cast<int>(storage.size()))
        {
            argv.reserve(storage.size() + 1);

            for (std::string& arg : storage)
            {
                argv.push_back(arg.data());
            }

            // 模拟真实 argv[argc] 结尾。
            argv.push_back(nullptr);
        }
    };
} // namespace

TEST(GetInputTest, ReadLongOptionEqualForm)
{
    ArgvCase args({
        "app",
        "--name=alice",
        "--other",
    });

    LOG_DEBUG("ReadLongOptionEqualForm");
    std::string output;
    const auto code =
        input_args::get_input(args.argc, args.argv.data(), "--name", output);

    EXPECT_EQ(error_code::ErrorCode::Ok, code);
    EXPECT_EQ("alice", output);

    // 被消费的选项应从 argv 中移除，未消费参数保持顺序。
    EXPECT_EQ(2, args.argc);
    EXPECT_STREQ("app", args.argv[0]);
    EXPECT_STREQ("--other", args.argv[1]);
    EXPECT_EQ(nullptr, args.argv[2]);
}

TEST(GetInputTest, ReadLongOptionSeparatedForm)
{
    ArgvCase args({
        "app",
        "--name",
        "alice",
        "--other",
    });

    std::string output;
    const auto code =
        input_args::get_input(args.argc, args.argv.data(), "--name", output);

    EXPECT_EQ(error_code::ErrorCode::Ok, code);
    EXPECT_EQ("alice", output);

    // 分离形式会同时移除选项名和值。
    EXPECT_EQ(2, args.argc);
    EXPECT_STREQ("app", args.argv[0]);
    EXPECT_STREQ("--other", args.argv[1]);
    EXPECT_EQ(nullptr, args.argv[2]);
}

TEST(GetInputTest, ReadShortOptionSeparatedForm)
{
    ArgvCase args({
        "app",
        "-n",
        "alice",
    });

    std::string output;
    const auto code =
        input_args::get_input(args.argc, args.argv.data(), "-n", output);

    EXPECT_EQ(error_code::ErrorCode::Ok, code);
    EXPECT_EQ("alice", output);

    EXPECT_EQ(1, args.argc);
    EXPECT_STREQ("app", args.argv[0]);
    EXPECT_EQ(nullptr, args.argv[1]);
}

TEST(GetInputTest, ReturnUnknownWhenOptionDoesNotExist)
{
    ArgvCase args({
        "app",
        "--other",
        "value",
    });

    std::string output = "unchanged";
    const auto code =
        input_args::get_input(args.argc, args.argv.data(), "--name", output);

    EXPECT_EQ(error_code::ErrorCode::Unknown, code);

    // 未找到选项时不改写 output 和 argv。
    EXPECT_EQ("unchanged", output);
    EXPECT_EQ(3, args.argc);
    EXPECT_STREQ("app", args.argv[0]);
    EXPECT_STREQ("--other", args.argv[1]);
    EXPECT_STREQ("value", args.argv[2]);
}

TEST(GetInputTest, ReturnUnknownWhenOptionHasNoValue)
{
    ArgvCase args({
        "app",
        "--name",
    });

    std::string output;
    const auto code =
        input_args::get_input(args.argc, args.argv.data(), "--name", output);

    EXPECT_EQ(error_code::ErrorCode::Unknown, code);
}

TEST(GetInputTest, ReturnParseFailedWhenNextArgumentLooksLikeOption)
{
    ArgvCase args({
        "app",
        "--name",
        "--other",
    });

    std::string output;
    const auto code =
        input_args::get_input(args.argc, args.argv.data(), "--name", output);

    // "--name --other" 中的 "--other" 更像另一个选项，不能当作 value。
    EXPECT_EQ(error_code::ErrorCode::ParseFailed, code);
}

TEST(GetInputTest, SupportMultipleCandidateOptionNames)
{
    ArgvCase args({
        "app",
        "-n=bob",
    });

    std::string output;
    const auto code =
        input_args::get_input(args.argc, args.argv.data(),
                              std::vector<std::string>{"--name", "-n"}, output);

    EXPECT_EQ(error_code::ErrorCode::Ok, code);
    EXPECT_EQ("bob", output);
}
