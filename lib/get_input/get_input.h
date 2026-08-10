#pragma once

#include <string>
#include <vector>
#include "error_code.h"

namespace input_args
{
    // 从 argc/argv 中读取指定选项值，支持 "--name value" 和 "--name=value" 两种形式。
    // 成功后会从 argv 中移除已消费的参数，并同步更新 argc。
    // 返回 Ok 表示读取成功，Unknown 表示未找到或缺少值，ParseFailed 表示后继参数看起来仍是选项。
    error_code::ErrorCode get_input(int& argc, char* argv[], const std::vector<std::string>& option_names,
                                    std::string& output);

    // 单个选项名的简化重载，例如只匹配 "--input"。
    error_code::ErrorCode get_input(int& argc, char* argv[], const std::string& option_name, std::string& output);
} // namespace input_args
