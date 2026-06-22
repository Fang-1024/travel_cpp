#include <cstdlib>
#include "log.h"
#include "error_code.h"
#include "get_input.h"

int main(int argc, char *argv[]) {
    LOG_INFO("Start, number of arg = %d.", argc);
    std::string target_arg{};
    const std::vector<std::string> input_options{"--test_mode", "--test_mode="};
    error_code::ErrorCode ret = input_args::get_input(argc, argv, input_options, target_arg);
    if (error_code::is_failed(ret)) {
        LOG_ERROR("Failed to parse arguments, ret = %s.", error_code::to_string(ret));
        return EXIT_FAILURE;
    }
    LOG_DEBUG("Parsing arguments, target_arg = %s.", target_arg.c_str());
    LOG_INFO("End.");
    return EXIT_SUCCESS;
}
