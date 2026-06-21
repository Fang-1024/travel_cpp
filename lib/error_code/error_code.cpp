#include "error_code.h"

namespace error_code {
    const char *to_string(const ErrorCode code) noexcept {
        switch (code) {
            case ErrorCode::Ok: return "Ok";
            case ErrorCode::Unknown: return "Unknown";
            case ErrorCode::NotImplemented: return "NotImplemented";
            case ErrorCode::InvalidArgument: return "InvalidArgument";
            case ErrorCode::NullPointer: return "NullPointer";
            case ErrorCode::OutOfRange: return "OutOfRange";
            case ErrorCode::Timeout: return "Timeout";
            case ErrorCode::Busy: return "Busy";

            case ErrorCode::NoMemory: return "NoMemory";
            case ErrorCode::NotFound: return "NotFound";
            case ErrorCode::AlreadyExists: return "AlreadyExists";
            case ErrorCode::PermissionDenied: return "PermissionDenied";
            case ErrorCode::IoError: return "IoError";

            case ErrorCode::ConfigMissing: return "ConfigMissing";
            case ErrorCode::ConfigInvalid: return "ConfigInvalid";
            case ErrorCode::ParseFailed: return "ParseFailed";
            case ErrorCode::SerializeFailed: return "SerializeFailed";
            case ErrorCode::DataCorrupted: return "DataCorrupted";

            case ErrorCode::ConnectionFailed: return "ConnectionFailed";
            case ErrorCode::Disconnected: return "Disconnected";
            case ErrorCode::SendFailed: return "SendFailed";
            case ErrorCode::ReceiveFailed: return "ReceiveFailed";
            case ErrorCode::ProtocolMismatch: return "ProtocolMismatch";

            case ErrorCode::BusinessRuleViolated: return "BusinessRuleViolated";
        }

        return "UnknownErrorCode";
    }
} // namespace learn_cpp::error_code
