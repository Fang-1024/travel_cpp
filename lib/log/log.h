#pragma once

// ============================================================================
// 目的：
// - 维持“C++ 工程化思想”：命名空间、enum class、RAII、线程安全、可配置、低开销
// - 但调用侧采用 C 风格占位符（printf/vsnprintf）来写日志
//
// 你将得到：
//   LOG_INFO("x=%d name=%s", x, name.c_str());
//   LOG_ERROR("open failed: %s", path.c_str());
//
// 注意：printf 风格不是类型安全的；为了减少误用：
// - 在 GCC/Clang 下使用 __attribute__((format(printf,...))) 做编译期格式检查
// - 在日志等级被过滤时，宏会避免求值参数（减少运行期开销）
// ============================================================================

// -----------------------------
// 标准库依赖
// -----------------------------
#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>

// printf/vsnprintf 所需
#include <cstdarg> // va_list, va_start, va_end, va_copy
#include <cstdio>  // std::vsnprintf

// ============================================================================
// 1) 函数签名宏 —— “在哪个函数里打印”
// ============================================================================
// 与你旧版一致：尽量在不同编译器上拿到“更可读的函数签名”
#if defined(__clang__) || defined(__GNUC__)
#define FUNC_SIGNATURE __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
#define FUNC_SIGNATURE __FUNCSIG__
#else
#define FUNC_SIGNATURE __func__
#endif

// ============================================================================
// 2) GCC/Clang：printf 格式串检查属性（非常推荐）
// ============================================================================
// __attribute__((format(printf, fmt_index, first_arg_index)))
// - fmt_index：格式串参数是第几个形参（从 1 开始计数）
// - first_arg_index：可变参数从第几个形参开始
//
// 这能在编译期发现类似：
//   LOG_INFO("x=%s", 123);  // 类型不匹配
//   LOG_INFO("x=%d %d", 1); // 参数数量不匹配
#if defined(__clang__) || defined(__GNUC__)
#define MINI_LOG_PRINTF_ATTR(fmt_index, first_arg_index) \
    __attribute__((format(printf, fmt_index, first_arg_index)))
#else
#define MINI_LOG_PRINTF_ATTR(fmt_index, first_arg_index)
#endif

namespace mini_log
{
    // =========================================================================
    // 3) 日志级别：强枚举（enum class）
    // =========================================================================
    enum class Level : int
    {
        Debug = 0,
        Info = 1,
        Warn = 2,
        Error = 3
    };

    // =========================================================================
    // 4) 编译期“最小输出级别”开关（与你旧版一致）
    // =========================================================================
    // 用宏实现“编译期默认值”：用户可在编译选项里覆盖：
    //   -DMINI_LOG_MIN_LEVEL=::mini_log::Level::Warn
#ifndef MINI_LOG_MIN_LEVEL
#define MINI_LOG_MIN_LEVEL ::mini_log::Level::Debug
#endif

    // =========================================================================
    // 5) should_log：在宏层面先判断，避免参数求值与格式化开销
    // =========================================================================
    // 关键点：
    // - 只要宏在 if(...) 分支里控制函数调用，那么当不满足时：
    //   - 不会进入函数
    //   - 也不会求值可变参数表达式（对“昂贵参数构造”非常关键）
    //
    // 例：
    //   LOG_DEBUG("big=%s", MakeHugeString().c_str());
    // 当 Debug 被过滤时，MakeHugeString() 不会被调用。
    constexpr bool should_log(Level lv) noexcept
    {
        return static_cast<int>(lv) >= static_cast<int>(MINI_LOG_MIN_LEVEL);
    }

    // =========================================================================
    // 6) 不同级别输出到哪个流（与你旧版一致）
    // =========================================================================
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

    // =========================================================================
    // 7) 时间字符串（到毫秒）—— 你旧版逻辑保留
    // =========================================================================
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

    // =========================================================================
    // 8) 线程 id 的“短显示”—— 你旧版逻辑保留
    // =========================================================================
    inline unsigned long thread_id_short()
    {
        std::ostringstream oss;
        oss << std::this_thread::get_id();
        unsigned long x = 0;
        std::istringstream(oss.str()) >> x;
        return x;
    }

    // =========================================================================
    // 9) 全局互斥锁：保证“整行日志原子输出”
    // =========================================================================
    // magic statics：C++11 起局部 static 初始化线程安全
    inline std::mutex& log_mutex()
    {
        static std::mutex m;
        return m;
    }

    // =========================================================================
    // 10) 将 printf 风格格式化为 std::string（核心）
    // =========================================================================
    //
    // vsnprintf 的语义：
    // - 返回“最终需要写出的字符数”（不含 '\0'）
    // - 如果缓冲区不够，它会截断，但返回值仍告诉你“需要多大”
    //
    // 我们采用“两段式”策略：
    // 1) 先用一个栈上小缓冲（避免频繁堆分配）
    // 2) 如果不够，再按需要大小堆分配一次
    inline std::string vformat_printf(const char* fmt, std::va_list ap)
    {
        if (!fmt) return std::string{"<null fmt>"};

        // ---- 1) 栈上小缓冲：大多数日志足够容纳 ----
        char stack_buf[512];

        // 注意：va_list 在被 vsnprintf 消费后不可复用，必须 va_copy
        std::va_list ap_copy;
        va_copy(ap_copy, ap);
        const int n1 = std::vsnprintf(stack_buf, sizeof(stack_buf), fmt, ap_copy);
        va_end(ap_copy);

        if (n1 < 0)
        {
            // 格式化失败（格式串错误等）
            return std::string{"<format error>"};
        }

        // n1 是“需要的字符数（不含 '\0'）”
        // 若 n1 < sizeof(stack_buf)，说明栈缓冲足够，stack_buf 内是完整字符串
        if (static_cast<std::size_t>(n1) < sizeof(stack_buf))
        {
            return std::string(stack_buf, static_cast<std::size_t>(n1));
        }

        // ---- 2) 堆分配：按需精确分配 n1 + 1（多一个给 '\0'）----
        std::string heap_buf;
        heap_buf.resize(static_cast<std::size_t>(n1) + 1); // 留 '\0' 的空间

        std::va_list ap_copy2;
        va_copy(ap_copy2, ap);
        const int n2 = std::vsnprintf(heap_buf.data(), heap_buf.size(), fmt, ap_copy2);
        va_end(ap_copy2);

        if (n2 < 0)
        {
            return std::string{"<format error>"};
        }

        // 去掉末尾 '\0'，让 string 的 size 恰好等于输出长度
        heap_buf.pop_back();
        return heap_buf;
    }

    // =========================================================================
    // 11) 最终输出：组装并打印一行日志（线程安全 + flush）
    // =========================================================================
    inline void log_impl(Level lv,
                         const char* file,
                         int line,
                         const char* func,
                         const std::string& text)
    {
        // 这里不再做 level 过滤：上层（宏/调用方）已经过滤过
        std::lock_guard lk(log_mutex());

        auto& os = stream_for(lv);

        os << '[' << now_string() << ']'
            << " [" << to_string(lv) << ']'
            << " [T" << thread_id_short() << ']'
            << ' ' << (file ? file : "<null-file>") << ':' << line
            << " | " << (func ? func : "<null-func>")
            << " | " << text
            << '\n';

        // 主动 flush：更利于崩溃前看到日志；代价是性能略低（教学/调试场景可接受）
        os.flush();
    }

    // =========================================================================
    // 12) 对外核心 API：logf（printf 风格，可变参数）
    // =========================================================================
    //
    // 设计点：
    // - 在函数内做 should_log 二次过滤（防御性）
    // - 先格式化，再统一交给 log_impl 输出
    // - GCC/Clang 下启用 printf 格式串编译期检查
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

    // =========================================================================
    // 13) 辅助：把“可输出到 ostream 的对象”转成 string（便于 %s）
    // =========================================================================
    // 用途：当你以前写 LOG_DEBUG(obj); 依赖 operator<< 时，
    // 现在可以写：
    //   LOG_DEBUG("%s", ::mini_log::to_string_stream(obj).c_str());
    template <class T>
    inline std::string to_string_stream(const T& v)
    {
        std::ostringstream oss;
        oss << v;
        return oss.str();
    }
} // namespace mini_log

// ============================================================================
// 14) 宏封装：调用侧只写 LOG_INFO("x=%d", x)
// ============================================================================
//
// 关键点：宏里先 should_log，再决定是否调用 mini_log::logf
// - 如果被过滤：不会调用 logf，也不会求值 __VA_ARGS__ 中的表达式
//
// 注意：下面的 LOG_xxx 都是 printf 风格：
//   LOG_INFO("id=%u name=%s", id, name.c_str());
//
#define LOG_AT_LEVEL(LV, ...) do {                                                     \
    if (::mini_log::should_log((LV)))                                                  \
        ::mini_log::logf((LV), __FILE__, __LINE__, FUNC_SIGNATURE, __VA_ARGS__);       \
} while (0)

#define LOG_DEBUG(...) LOG_AT_LEVEL(::mini_log::Level::Debug, __VA_ARGS__)
#define LOG_INFO(...)  LOG_AT_LEVEL(::mini_log::Level::Info,  __VA_ARGS__)
#define LOG_WARN(...)  LOG_AT_LEVEL(::mini_log::Level::Warn,  __VA_ARGS__)
#define LOG_ERROR(...) LOG_AT_LEVEL(::mini_log::Level::Error, __VA_ARGS__)