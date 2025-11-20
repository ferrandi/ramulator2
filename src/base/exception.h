#ifndef RAMULATOR_BASE_EXCEPTION_H
#define RAMULATOR_BASE_EXCEPTION_H

#include <stdexcept>
#include <string>
#include <utility>

namespace Ramulator
{

struct InitializationError : public std::logic_error
{
    explicit InitializationError(const std::string& message)
        : std::logic_error(message) {}

    // optional convenience overload for move semantics
    explicit InitializationError(std::string&& message)
        : std::logic_error(std::move(message)) {}
};

struct ConfigurationError : public std::runtime_error
{
    explicit ConfigurationError(const std::string& message)
        : std::runtime_error(message) {}

    explicit ConfigurationError(std::string&& message)
        : std::runtime_error(std::move(message)) {}
};

} // namespace Ramulator

#endif // RAMULATOR_BASE_EXCEPTION_H