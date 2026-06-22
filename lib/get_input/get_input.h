#pragma once

#include <string>
#include <vector>
#include "error_code.h"

namespace input_args {
    // 从 argc/argv 中查找指定参数，并把找到的值写入 output。
    //
    // 支持两种形式：
    //   --name value
    //   --name=value
    //
    // 也支持短参数：
    //   -n value
    //   -n=value
    //
    // 如果找到并成功读取值，会从 argv 中剥离已处理参数，并同步更新 argc。
    //
    // 返回值：
    //   true  : 找到参数，并成功读取 value
    //   false : 没找到参数，或者参数后缺少 value
    error_code::ErrorCode get_input(int &argc, char *argv[], const std::vector<std::string> &option_names,
                                    std::string &output);

    // 单个参数名的简化重载，例如只匹配 "--input"
    error_code::ErrorCode get_input(int &argc, char *argv[], const std::string &option_name, std::string &output);
}
