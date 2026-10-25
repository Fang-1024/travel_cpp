// 源文件首先包含自己的公开头，可尽早暴露头文件缺少必要 include 的问题。
#include <travel_cpp/log.h>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace mini_log
{
    Logger::Logger(const std::string& name, const Level min_level) : name_(name), min_level_(min_level)
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

    void Logger::log(const Level current_level, const std::string& message) const
    {
        // 若等级不满足打印要求，则直接跳过
        if (!should_log(current_level))
        {
            return;
        }

        std::ostream& output = current_level == Level::Warn || current_level == Level::Error ? std::cerr : std::cout;
        output
            << " [" << name_ << "] "
            << message
            << '\n';
        output.flush();
    }

    // 修改日志等级
    void Logger::set_min_level(const Level level)
    {
        min_level_ = level;
    }

    // 查询日志等级
    Level Logger::min_level() const
    {
        return min_level_;
    }

    bool Logger::should_log(const Level current_level) const
    {
        return min_level_ <= current_level;
    }
}
