#include <cstdlib>

// 从 CMake 公开的 include/ 根目录开始包含，项目前缀可避免与第三方 log.h 重名。
#include <travel_cpp/error_code.h>
#include <travel_cpp/get_input.h>
#include <travel_cpp/log.h>

int main(int argc, char* argv[])
{
    LOG_INFO("Start, number of arg = %d.", argc);
    LOG_INFO("End.");
    return EXIT_SUCCESS;
}
