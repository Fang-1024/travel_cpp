#pragma once
#include <string>

namespace mini_log
{
    enum class Level
    {
        Debug = 0,
        Info = 1,
        Warn = 2,
        Error = 3
    };

    class Logger
    {
    public:
        // 构造函数。
        //
        // name      : Logger 的名称，例如 "app"、"network"。
        // min_level : 最低输出等级，默认输出 Debug 及以上日志。
        explicit Logger(const std::string& name);

        // 对外提供的日志接口。
        void debug(const std::string& message) const;
        void info(const std::string& message) const;
        void warn(const std::string& message) const;
        void error(const std::string& message) const;

    private:
        // Logger 对象自己的状态。
        std::string name_;

        // 真正执行日志输出的内部函数。
        //
        // 用户不需要直接调用它，所以放在 private 中。
        void log(Level currrent_level, const std::string& message) const;
    };
}
