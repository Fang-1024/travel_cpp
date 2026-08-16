#pragma once

// 轻量 printf 风格日志工具。
// 宏会记录源码位置，并在日志级别被过滤时避免求值格式化参数。
// 示例：LOG_INFO("id=%d name=%s", id, name.c_str());
#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>

#include <cstdarg> // va_list, va_start, va_end, va_copy
#include <cstdio>  // std::vsnprintf

// 在不同编译器上尽量取得更可读的函数签名。
#if defined(__clang__) || defined(__GNUC__)
#define FUNC_SIGNATURE __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
#define FUNC_SIGNATURE __FUNCSIG__
#else
#define FUNC_SIGNATURE __func__
#endif

// GCC/Clang 下启用 printf 格式串检查，尽早发现占位符和参数不匹配。
#if defined(__clang__) || defined(__GNUC__)
#define MINI_LOG_PRINTF_ATTR(fmt_index, first_arg_index) \
    __attribute__((format(printf, fmt_index, first_arg_index)))
#else
#define MINI_LOG_PRINTF_ATTR(fmt_index, first_arg_index)
#endif

namespace mini_log
{
    enum class Level : int
    {
        Debug = 0,
        Info = 1,
        Warn = 2,
        Error = 3
    };

    // 可通过编译选项覆盖最小输出级别，例如：
    //   -DMINI_LOG_MIN_LEVEL=::mini_log::Level::Warn
#ifndef MINI_LOG_MIN_LEVEL
#define MINI_LOG_MIN_LEVEL ::mini_log::Level::Debug
#endif

    // 供宏先判断日志级别，避免被过滤日志的参数求值和格式化开销。
    constexpr bool should_log(Level lv) noexcept
    {
        return static_cast<int>(lv) >= static_cast<int>(MINI_LOG_MIN_LEVEL);
    }

    inline std::ostream& stream_for(const Level lv)
    {
        switch (lv)
        {
        case Level::Error:
        case Level::Warn:
            return std::cerr;
        default:
            return std::cout;
        }
    }

    inline const char* to_string(const Level lv)
    {
        switch (lv)
        {
        case Level::Debug: return "DEBUG";
        case Level::Info: return "INFO";
        case Level::Warn: return "WARN";
        case Level::Error: return "ERROR";
        }
        return "?";
    }

    // 使用线程安全的 localtime_s/localtime_r，保证多线程日志时间戳可靠。
    inline std::string now_string()
    {
        const auto tp = std::chrono::system_clock::now();
        const auto secs = std::chrono::time_point_cast<std::chrono::seconds>(tp);
        const auto ms =
            std::chrono::duration_cast<std::chrono::milliseconds>(tp - secs).count();

        std::time_t tt = std::chrono::system_clock::to_time_t(tp);

        std::tm tm_snapshot{};
#if defined(_WIN32)
        localtime_s(&tm_snapshot, &tt);
#else
        localtime_r(&tt, &tm_snapshot);
#endif

        std::ostringstream oss;
        oss << std::put_time(&tm_snapshot, "%Y-%m-%d %H:%M:%S")
            << '.'
            << std::setfill('0') << std::setw(3) << ms;
        return oss.str();
    }

    // 仅用于日志显示，不承诺等同于操作系统线程 ID。
    inline unsigned long thread_id_short()
    {
        std::ostringstream oss;
        oss << std::this_thread::get_id();
        unsigned long x = 0;
        std::istringstream(oss.str()) >> x;
        return x;
    }

    // 保护整行日志输出，避免多线程写同一行时相互穿插。
    inline std::mutex& log_mutex()
    {
        static std::mutex m;
        return m;
    }

    // 先尝试栈缓冲，放不下时再按 vsnprintf 返回的准确长度分配堆缓冲。
    inline std::string vformat_printf(const char* fmt, std::va_list ap)
    {
        if (!fmt) return std::string{"<null fmt>"};

        char stack_buf[512];

        // va_list 被 vsnprintf 消费后不可复用，重复格式化前必须 va_copy。
        std::va_list ap_copy;
        va_copy(ap_copy, ap);
        const int n1 = std::vsnprintf(stack_buf, sizeof(stack_buf), fmt, ap_copy);
        va_end(ap_copy);

        if (n1 < 0)
        {
            return std::string{"<format error>"};
        }

        if (static_cast<std::size_t>(n1) < sizeof(stack_buf))
        {
            return std::string(stack_buf, static_cast<std::size_t>(n1));
        }

        std::string heap_buf;
        heap_buf.resize(static_cast<std::size_t>(n1) + 1);

        std::va_list ap_copy2;
        va_copy(ap_copy2, ap);
        const int n2 = std::vsnprintf(heap_buf.data(), heap_buf.size(), fmt, ap_copy2);
        va_end(ap_copy2);

        if (n2 < 0)
        {
            return std::string{"<format error>"};
        }

        // 去掉为 '\0' 预留的位置，使 size 等于真实输出长度。
        heap_buf.pop_back();
        return heap_buf;
    }

    inline void log_impl(Level lv,
                         const char* file,
                         int line,
                         const char* func,
                         const std::string& text)
    {
        std::lock_guard lk(log_mutex());

        auto& os = stream_for(lv);

        os << '[' << now_string() << ']'
            << " [" << to_string(lv) << ']'
            << " [T" << thread_id_short() << ']'
            << ' ' << (file ? file : "<null-file>") << ':' << line
            << " | " << (func ? func : "<null-func>")
            << " | " << text
            << '\n';

        // demo/调试场景优先保证崩溃前日志可见。
        os.flush();
    }

    // printf 风格日志入口；宏通常会先过滤一次，这里再做防御性检查。
    inline void logf(Level lv,
                     const char* file,
                     int line,
                     const char* func,
                     const char* fmt, ...) MINI_LOG_PRINTF_ATTR(5, 6);

    inline void logf(Level lv,
                     const char* file,
                     int line,
                     const char* func,
                     const char* fmt, ...)
    {
        if (!should_log(lv))
            return;

        std::va_list ap;
        va_start(ap, fmt);
        std::string msg = vformat_printf(fmt, ap);
        va_end(ap);

        log_impl(lv, file, line, func, msg);
    }

    // 便于把支持 operator<< 的对象转给 printf 风格日志的 %s。
    template <class T>
    inline std::string to_string_stream(const T& v)
    {
        std::ostringstream oss;
        oss << v;
        return oss.str();
    }
} // namespace mini_log

// 宏先做级别过滤，再调用 logf；被过滤时不会求值 __VA_ARGS__。
#define LOG_AT_LEVEL(LV, ...) do {                                                     \
    if (::mini_log::should_log((LV)))                                                  \
        ::mini_log::logf((LV), __FILE__, __LINE__, FUNC_SIGNATURE, __VA_ARGS__);       \
} while (0)

#define LOG_DEBUG(...) LOG_AT_LEVEL(::mini_log::Level::Debug, __VA_ARGS__)
#define LOG_INFO(...)  LOG_AT_LEVEL(::mini_log::Level::Info,  __VA_ARGS__)
#define LOG_WARN(...)  LOG_AT_LEVEL(::mini_log::Level::Warn,  __VA_ARGS__)
#define LOG_ERROR(...) LOG_AT_LEVEL(::mini_log::Level::Error, __VA_ARGS__)
