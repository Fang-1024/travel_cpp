#pragma once

#include <string>

namespace mini_log
{
    // 日志等级。
    // 使用 enum class 可以避免普通 enum 带来的名称污染和隐式整数转换问题。
    enum class Level : int
    {
        Debug = 0,
        Info = 1,
        Warn = 2,
        Error = 3
    };

    // Logger 表示一个“日志记录器”对象。
    //
    // 一个 Logger 对象具有自己的：
    //   1. 名称
    //   2. 最小日志输出等级
    //
    // 并提供 debug/info/warn/error 等行为。
    class Logger
    {
    public:
        // 构造函数。
        //
        // name      : Logger 的名称，例如 "app"、"network"。
        // min_level : 最低输出等级，默认输出 Debug 及以上日志。
        explicit Logger(const std::string& name, Level min_level = Level::Debug);

        // 修改最低日志输出等级。
        void set_min_level(Level level);

        // 查询最低日志输出等级。
        Level min_level() const;

        // 对外提供的日志接口。
        void debug(const std::string& message) const;
        void info(const std::string& message) const;
        void warn(const std::string& message) const;
        void error(const std::string& message) const;

    private:
        // Logger 对象自己的状态。
        std::string name_;
        Level min_level_;

        // 真正执行日志输出的内部函数。
        //
        // 用户不需要直接调用它，所以放在 private 中。
        void log(Level level, const std::string& message) const;

        // 判断某个日志等级是否应该输出。
        bool should_log(Level level) const;

        // 把日志等级转换成字符串。
        static const char* level_to_string(Level level);

        // 获取当前时间字符串。
        static std::string now_string();
    };
}
