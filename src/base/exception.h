#ifndef     RAMULATOR_BASE_EXCEPTION_H
#define     RAMULATOR_BASE_EXCEPTION_H

#include <stdexcept>
#include <string>
#include <string_view>

#include "base/simple_logging.h"

namespace Ramulator {

struct InitializationError : public std::logic_error {
  template <typename... Args>
  InitializationError(const std::string& format_str, Args&&... args) : std::logic_error(fmt::format(format_str, args...)){};
  InitializationError(const std::string& msg) : std::logic_error(msg){};
};

struct ConfigurationError : public std::runtime_error {
  template <typename... Args>
  ConfigurationError(const std::string& format_str, Args&&... args) : std::runtime_error(fmt::format(format_str, args...)){};
  ConfigurationError(const std::string& msg) : std::runtime_error(msg){};
};

}        // namespace Ramulator


#endif   // RAMULATOR_BASE_EXCEPTION_H