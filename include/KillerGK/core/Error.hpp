/**
 * @file Error.hpp
 * @brief Error handling types for KillerGK
 */

#pragma once

#include <string>
#include <functional>
#include <optional>

namespace KillerGK {

/**
 * @enum ErrorCode
 * @brief Error codes for KillerGK operations
 */
enum class ErrorCode {
    Success = 0,

    // Initialization errors
    VulkanNotAvailable,
    InsufficientGPUMemory,
    MissingDependency,

    // Resource errors
    FileNotFound,
    InvalidFormat,
    CorruptedData,
    OutOfMemory,

    // Runtime errors
    InvalidState,
    ConstraintConflict,
    InvalidParameter,

    // Network errors
    ConnectionFailed,
    Timeout,

    // Platform errors
    FeatureNotAvailable,
    PermissionDenied
};

/**
 * @class Error
 * @brief Error information container
 */
class Error {
public:
    ErrorCode code = ErrorCode::Success;
    std::string message;
    std::string file;
    int line = 0;

    Error() = default;
    Error(ErrorCode code, std::string message = "", std::string file = "", int line = 0)
        : code(code), message(std::move(message)), file(std::move(file)), line(line) {}

    [[nodiscard]] bool isSuccess() const { return code == ErrorCode::Success; }
    explicit operator bool() const { return !isSuccess(); }
};

/**
 * @class Result
 * @brief Result type for operations that can fail
 */
template<typename T>
class Result {
public:
    Result(T value) : m_value(std::move(value)) {}
    Result(Error error) : m_error(std::move(error)) {}

    [[nodiscard]] bool hasValue() const { return m_value.has_value(); }
    [[nodiscard]] bool hasError() const { return m_error.has_value(); }

    T& value() { return *m_value; }
    const T& value() const { return *m_value; }

    Error& error() { return *m_error; }
    const Error& error() const { return *m_error; }

    T valueOr(T defaultValue) const {
        return hasValue() ? *m_value : defaultValue;
    }

private:
    std::optional<T> m_value;
    std::optional<Error> m_error;
};

// Specialization for void
template<>
class Result<void> {
public:
    Result() = default;
    Result(Error error) : m_error(std::move(error)) {}

    [[nodiscard]] bool hasError() const { return m_error.has_value(); }
    [[nodiscard]] bool isSuccess() const { return !hasError(); }

    Error& error() { return *m_error; }
    const Error& error() const { return *m_error; }

private:
    std::optional<Error> m_error;
};

/**
 * @brief Global error callback type
 */
using ErrorCallback = std::function<void(const Error&)>;

/**
 * @brief Set global error callback
 */
void setErrorCallback(ErrorCallback callback);

/**
 * @enum LogLevel
 * @brief Logging levels
 */
enum class LogLevel { 
    Debug, 
    Info, 
    Warning, 
    Error, 
    Fatal 
};

/**
 * @brief Set minimum log level
 */
void setLogLevel(LogLevel level);

/**
 * @brief Log a message
 */
void log(LogLevel level, const std::string& message);

} // namespace KillerGK
