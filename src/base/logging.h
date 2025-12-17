#ifndef RAMULATOR_BASE_LOGGING_H
#define RAMULATOR_BASE_LOGGING_H

#include <yaml-cpp/yaml.h>

#include <string>
#include <vector>

#include "base/exception.h"

class DummyLogger {
  public:
    template <typename... Args>
    void log_info(const std::string&, Args&&...) {}
    template <typename... Args>
    void log_warn(const std::string&, Args&&...) {}
    template <typename... Args>
    void log_error(const std::string&, Args&&...) {}
    template <typename... Args>
    void log_debug(const std::string&, Args&&...) {}
};

// TODO: Better Logging interface. Put logging methods into Implementation base class?
namespace Ramulator
{

using Logger_t = std::shared_ptr<DummyLogger>;

class Logging
{
  private:
    inline static const std::string default_logger_pattern = "[%n] [%l] %v";

  public:
    /**
     * @brief       Create a logger.
     *
     * @param name  The name of the logger
     * @return Logger_t
     */
    static Logger_t create_logger(std::string name, std::string pattern = default_logger_pattern);

    /**
     * @brief       Returns a pointer to the logger by its name.
     *
     * @param name
     * @return Logger_t
     */
    static Logger_t get(std::string name);

  private:
    static bool _create_base_logger();
    inline static bool base_logger_registered = _create_base_logger();

  public:
    Logging() = delete;
    Logging(const Logging&) = delete;
    void operator=(const Logging&) = delete;
    Logging(Logging&&) = delete;
};

} // namespace Ramulator

#endif // RAMULATOR_BASE_LOGGING_H