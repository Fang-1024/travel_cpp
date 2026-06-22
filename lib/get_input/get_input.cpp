#include <string>
#include <vector>
#include "get_input.h"
#include "error_code.h"

namespace {
    bool starts_with(const std::string &text, const std::string &prefix) {
        if (text.size() < prefix.size()) {
            return false;
        }

        return text.compare(0, prefix.size(), prefix) == 0;
    }

    bool is_option_like(const std::string &text) {
        // 简单判断是否像一个命令行选项。
        // 这样可以避免 "--a --b value" 时，把 "--b" 错误当成 --a 的值。
        //
        // 如果 value 本身确实以 '-' 开头，建议使用：
        //   --num=-1
        return text.size() >= 2 && text[0] == '-';
    }

    bool try_get_value_from_equal_form(const std::string &arg, const std::string &option_name,
                                       std::string &value) {
        const std::string prefix = option_name + "=";

        if (!starts_with(arg, prefix)) {
            return false;
        }

        value = arg.substr(prefix.size());
        return true;
    }

    void erase_argv_range(int &argc, char *argv[], const int erase_begin, const int erase_count) {
        if (erase_count <= 0) {
            return;
        }

        int write_index = erase_begin;
        int read_index = erase_begin + erase_count;

        while (read_index < argc) {
            argv[write_index] = argv[read_index];
            ++write_index;
            ++read_index;
        }

        argc = write_index;

        // argv[argc] 按惯例应为 nullptr。
        // 这里主动维护，方便后续流程按 C 风格遍历 argv。
        argv[argc] = nullptr;
    }
}

namespace input_args {
    error_code::ErrorCode get_input(int &argc, char *argv[], const std::vector<std::string> &option_names,
                                    std::string &output) {
        if (argc <= 0 || argv == nullptr || option_names.empty()) {
            return error_code::ErrorCode::InvalidArgument;
        }

        int found_index = -1;
        int erase_count = 0;
        std::string found_value{};

        // argv[0] 是程序名，从 argv[1] 开始解析。
        for (int i = 1; i < argc; ++i) {
            if (argv[i] == nullptr) {
                continue;
            }

            const std::string current_arg = argv[i];

            for (const std::string &option_name: option_names) {
                if (option_name.empty()) {
                    continue;
                }

                // 形式一：--name=value 或 -n=value
                std::string value_from_equal;
                if (try_get_value_from_equal_form(current_arg, option_name, value_from_equal)) {
                    found_index = i;
                    erase_count = 1;
                    found_value = value_from_equal;
                    break;
                }

                // 形式二：--name value 或 -n value
                if (current_arg == option_name) {
                    if (i + 1 >= argc || argv[i + 1] == nullptr) {
                        return error_code::ErrorCode::Unknown;
                    }

                    const std::string next_arg = argv[i + 1];

                    if (is_option_like(next_arg)) {
                        return error_code::ErrorCode::ParseFailed;
                    }

                    found_index = i;
                    erase_count = 2;
                    found_value = next_arg;
                    break;
                }
            }

            if (found_index >= 0) {
                break;
            }
        }

        if (found_index < 0) {
            return error_code::ErrorCode::Unknown;
        }

        output = found_value;
        erase_argv_range(argc, argv, found_index, erase_count);
        return error_code::ErrorCode::Ok;
    }

    error_code::ErrorCode get_input(int &argc, char *argv[], const std::string &option_name, std::string &output) {
        return get_input(argc, argv, std::vector<std::string>{option_name}, output);
    }
}
