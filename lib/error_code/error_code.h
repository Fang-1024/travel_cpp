#ifndef LEARN_CPP_ERROR_CODE_H
#define LEARN_CPP_ERROR_CODE_H

#include <cstdint>

namespace error_code {
    // 项目统一状态码：0 表示成功，非 0 表示按区间分组的失败原因。
    enum class ErrorCode : std::int32_t {
        // 通用错误：0 ~ 99。
        Ok = 0,
        Unknown = 1,
        NotImplemented = 2,
        InvalidArgument = 3,
        NullPointer = 4,
        OutOfRange = 5,
        Timeout = 6,
        Busy = 7,

        // 资源和系统错误：100 ~ 199。
        NoMemory = 100,
        NotFound = 101,
        AlreadyExists = 102,
        PermissionDenied = 103,
        IoError = 104,

        // 配置和数据错误：200 ~ 299。
        ConfigMissing = 200,
        ConfigInvalid = 201,
        ParseFailed = 202,
        SerializeFailed = 203,
        DataCorrupted = 204,

        // 通信和网络错误：300 ~ 399。
        ConnectionFailed = 300,
        Disconnected = 301,
        SendFailed = 302,
        ReceiveFailed = 303,
        ProtocolMismatch = 304,

        // 业务错误：1000+。
        BusinessRuleViolated = 1000,
    };

    constexpr bool is_ok(const ErrorCode code) noexcept {
        return code == ErrorCode::Ok;
    }

    constexpr bool is_failed(const ErrorCode code) noexcept {
        return !is_ok(code);
    }

    // 用于日志和测试断言的稳定文本。
    const char *to_string(ErrorCode code) noexcept;
} // namespace error_code

#endif //LEARN_CPP_ERROR_CODE_H
