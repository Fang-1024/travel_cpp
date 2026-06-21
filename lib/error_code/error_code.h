#ifndef LEARN_CPP_ERROR_CODE_H
#define LEARN_CPP_ERROR_CODE_H

// 当前就暂时使用OK和False来返回所有状态
#include <cstdint>

namespace error_code {
    // 项目统一故障码：
    // 1) 0 表示成功；
    // 2) 非 0 表示失败；
    // 3) 按区间分组，便于新手快速定位问题类型。
    enum class ErrorCode : std::int32_t {
        // -------------------
        // 通用（0 ~ 99）
        // -------------------
        Ok = 0,
        Unknown = 1,
        NotImplemented = 2,
        InvalidArgument = 3,
        NullPointer = 4,
        OutOfRange = 5,
        Timeout = 6,
        Busy = 7,

        // -------------------
        // 资源/系统（100 ~ 199）
        // -------------------
        NoMemory = 100,
        NotFound = 101,
        AlreadyExists = 102,
        PermissionDenied = 103,
        IoError = 104,

        // -------------------
        // 配置/数据（200 ~ 299）
        // -------------------
        ConfigMissing = 200,
        ConfigInvalid = 201,
        ParseFailed = 202,
        SerializeFailed = 203,
        DataCorrupted = 204,

        // -------------------
        // 通信/网络（300 ~ 399）
        // -------------------
        ConnectionFailed = 300,
        Disconnected = 301,
        SendFailed = 302,
        ReceiveFailed = 303,
        ProtocolMismatch = 304,

        // -------------------
        // 业务（1000+）
        // -------------------
        BusinessRuleViolated = 1000,
    };

    // 建议在业务代码里直接使用这两个辅助函数，降低“魔法数字”与误用。
    constexpr bool is_ok(const ErrorCode code) noexcept {
        return code == ErrorCode::Ok;
    }

    constexpr bool is_failed(const ErrorCode code) noexcept {
        return !is_ok(code);
    }

    // 故障码 -> 可读字符串（用于日志/调试）
    const char *to_string(ErrorCode code) noexcept;
} // namespace learn_cpp::error_code

#endif //LEARN_CPP_ERROR_CODE_H