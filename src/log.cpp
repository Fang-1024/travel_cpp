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


    void Logger::set_min_level(const Level level)
    {
        min_level_ = level;
    }


    Level Logger::min_level() const
    {
        return min_level_;
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


    bool Logger::should_log(Level level) const
    {
        return static_cast<int>(level) >= static_cast<int>(min_level_);
    }


    void Logger::log(const Level level, const std::string& message) const
    {
        // 如果低于 Logger 当前设定的最低等级，就直接忽略。
        if (!should_log(level))
        {
            return;
        }

        // Warn/Error 输出到 stderr，
        // Debug/Info 输出到 stdout。
        std::ostream& output = (level == Level::Warn || level == Level::Error) ? std::cerr : std::cout;

        output
            << '[' << now_string() << ']'
            << " [" << level_to_string(level) << ']'
            << " [" << name_ << "] "
            << message
            << '\n';

        output.flush();
    }


    const char* Logger::level_to_string(const Level level)
    {
        switch (level)
        {
        case Level::Debug:
            return "DEBUG";

        case Level::Info:
            return "INFO";

        case Level::Warn:
            return "WARN";

        case Level::Error:
            return "ERROR";
        }

        return "UNKNOWN";
    }


    std::string Logger::now_string()
    {
        const auto now = std::chrono::system_clock::now();

        const std::time_t time = std::chrono::system_clock::to_time_t(now);

        std::tm time_info{};

#if defined(_WIN32)
        localtime_s(&time_info, &time);
#else
        localtime_r(&time, &time_info);
#endif

        std::ostringstream oss;

        oss << std::put_time(&time_info, "%Y-%m-%d %H:%M:%S");

        return oss.str();
    }
} // namespace mini_log
