#include <cstdlib>
#include <string>
// 从 CMake 公开的 include/ 根目录开始包含，项目前缀可避免与第三方 log.h 重名。
#include <travel_cpp/log.h>

int main(int argc, char* argv[])
{
    mini_log::Logger logger("app");
    logger.info("Start, number of arg = " + std::to_string(argc) + ".");
    logger.info("End.");
    return EXIT_SUCCESS;
}
