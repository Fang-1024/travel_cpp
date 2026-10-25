#include <cstdlib>
#include <string>
// 从 CMake 公开的 include/ 根目录开始包含，项目前缀可避免与第三方 log.h 重名。
#include <iostream>
#include <ostream>
#include <travel_cpp/log.h>

int main(int argc, char* argv[])
{
    mini_log::Logger app(
        "app",
        mini_log::Level::Info);

    mini_log::Logger network(
        "network",
        mini_log::Level::Debug);

    app.info("Application started");
    network.debug("Connetting started");

    app.set_min_level(mini_log::Level::Warn);
    app.debug("Application restarted");

    return EXIT_SUCCESS;
}
