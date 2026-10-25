// 源文件首先包含自己的公开头，可尽早暴露头文件缺少必要 include 的问题。
#include <travel_cpp/log.h>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace mini_log
{
    Logger::Logger(const std::string& name) : name_(name)
    {
    }

    void Logger::debug(const std::string& message) const
    {
        log(Level::Debug, message);
    }

    void Logger::info(const std::string& message) const
    {
        log(Level::Info, message);
    }

    void Logger::warn(const std::string& message) const
    {
        log(Level::Warn, message);
    }

    void Logger::error(const std::string& message) const
    {
        log(Level::Error, message);
    }

    void Logger::log(Level currrent_level, const std::string& message) const
    {
        std::ostream& output = (currrent_level == Level::Warn || currrent_level == Level::Error)
                                   ? std::cerr
                                   : std::cout;
        output
            << " [" << name_ << "] "
            << message
            << '\n';
        output.flush();
    }
}
