// SPDX-License-Identifier: MIT
// Result<T, E> - Error handling pattern for exception-free C++
// MISRA C++: Exception-free, uses Result pattern instead

#pragma once

#include <optional>
#include <type_traits>
#include <utility>
#include <variant>

namespace configgui {
namespace core {

/// @brief Result<T, E> type for error handling without exceptions
/// @tparam T Success type
/// @tparam E Error type
/// 
/// OPTIMIZATION: Fully optimized for move semantics
/// - Copy constructors for lvalue references (cheap for small types)
/// - Move constructors for rvalue references (efficient for large types)
/// - Ref-qualified member functions enable RVO for moveable types
/// - std::variant provides efficient storage with no overhead
template <typename T, typename E>
class Result
{
public:
    /// @brief Construct a success result (copy)
    explicit Result(const T& value) : data_(value) {}

    /// @brief Construct a success result (move)
    /// OPTIMIZATION: Move semantics for efficient transfer of large objects
    explicit Result(T&& value) : data_(std::move(value)) {}

    /// @brief Construct a failure result (copy)
    explicit Result(const E& error) : data_(error) {}

    /// @brief Construct a failure result (move)
    /// OPTIMIZATION: Move semantics for efficient error object transfer
    explicit Result(E&& error) : data_(std::move(error)) {}

    /// @brief Check if result is success
    [[nodiscard]] bool is_success() const { return std::holds_alternative<T>(data_); }

    /// @brief Check if result is failure
    [[nodiscard]] bool is_failure() const { return std::holds_alternative<E>(data_); }

    /// @brief Get success value (lvalue ref, unsafe)
    [[nodiscard]] T& value() & { return std::get<T>(data_); }

    /// @brief Get success value (const ref, unsafe)
    [[nodiscard]] const T& value() const& { return std::get<T>(data_); }

    /// @brief Get success value (rvalue ref, unsafe)
    /// OPTIMIZATION: Ref-qualified member enables RVO for move operations
    [[nodiscard]] T&& value() && { return std::get<T>(std::move(data_)); }

    /// @brief Get error value (lvalue ref, unsafe)
    [[nodiscard]] E& error() & { return std::get<E>(data_); }

    /// @brief Get error value (const ref, unsafe)
    [[nodiscard]] const E& error() const& { return std::get<E>(data_); }

    /// @brief Get error value (rvalue ref, unsafe)
    /// OPTIMIZATION: Ref-qualified member enables RVO for move operations
    [[nodiscard]] E&& error() && { return std::get<E>(std::move(data_)); }

    /// @brief Get success value or default
    [[nodiscard]] T value_or(const T& default_value) const
    {
        return is_success() ? std::get<T>(data_) : default_value;
    }

private:
    std::variant<T, E> data_;
};

} // namespace core
} // namespace configgui
