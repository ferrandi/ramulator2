#ifndef RAMULATOR_SIMPLE_LOGGING_H
#define RAMULATOR_SIMPLE_LOGGING_H

// Simple logging replacement for spdlog
// Provides basic logging functionality without external dependencies

#include <iostream>
#include <memory>
#include <string>
#include <cstdio>
#include <sstream>

// Temporarily disable debug/error/info/warn macros from MDPI
#ifdef debug
#pragma push_macro("debug")
#undef debug
#define RAMULATOR_HAD_DEBUG_MACRO
#endif

#ifdef info
#pragma push_macro("info")
#undef info
#define RAMULATOR_HAD_INFO_MACRO
#endif

#ifdef warn
#pragma push_macro("warn")
#undef warn
#define RAMULATOR_HAD_WARN_MACRO
#endif

#ifdef error
#pragma push_macro("error")
#undef error
#define RAMULATOR_HAD_ERROR_MACRO
#endif

namespace Ramulator {

// Simple logger class to replace spdlog::logger
class SimpleLogger {
private:
    std::string name_;

public:
    SimpleLogger(const std::string& name) : name_(name) {}

    void debug(const std::string& format) {
        std::fprintf(stderr, "[DEBUG][%s] %s\n", name_.c_str(), format.c_str());
    }

    void info(const std::string& format) {
        std::fprintf(stderr, "[INFO][%s] %s\n", name_.c_str(), format.c_str());
    }

    void warn(const std::string& format) {
        std::fprintf(stderr, "[WARN][%s] %s\n", name_.c_str(), format.c_str());
    }

    void error(const std::string& format) {
        std::fprintf(stderr, "[ERROR][%s] %s\n", name_.c_str(), format.c_str());
    }

    // Additional overloads for common patterns we found in Ramulator2 code
    template<typename T1, typename T2, typename T3>
    void warn(const std::string& format, T1 arg1, T2 arg2, T3 arg3) {
        std::fprintf(stderr, "[WARN][%s] %s\n", name_.c_str(), format.c_str());
    }

    template<typename T1, typename T2>
    void info(const std::string& format, T1 arg1, T2 arg2) {
        std::fprintf(stderr, "[INFO][%s] %s\n", name_.c_str(), format.c_str());
    }

    template<typename T1>
    void info(const std::string& format, T1 arg1) {
        std::fprintf(stderr, "[INFO][%s] %s\n", name_.c_str(), format.c_str());
    }

    template<typename T1, typename T2, typename T3>
    void debug(const std::string& format, T1 arg1, T2 arg2, T3 arg3) {
        std::fprintf(stderr, "[DEBUG][%s] %s\n", name_.c_str(), format.c_str());
    }

    void set_pattern(const std::string&) {}
    void set_level(int) {}
};

typedef std::shared_ptr<SimpleLogger> logger_ptr;

// Simple replacement functions for spdlog namespace functions
namespace spdlog {
    inline logger_ptr stdout_color_st(const std::string& name) {
        return std::make_shared<SimpleLogger>(name);
    }

    inline logger_ptr get(const std::string& name) {
        return std::make_shared<SimpleLogger>(name);
    }

    namespace level {
        enum level_enum { debug = 0, info = 1, warn = 2, err = 3 };
    }
}

} // namespace Ramulator

// Restore the macros using pragma
#ifdef RAMULATOR_HAD_DEBUG_MACRO
#pragma pop_macro("debug")
#undef RAMULATOR_HAD_DEBUG_MACRO
#endif

#ifdef RAMULATOR_HAD_INFO_MACRO
#pragma pop_macro("info")
#undef RAMULATOR_HAD_INFO_MACRO
#endif

#ifdef RAMULATOR_HAD_WARN_MACRO
#pragma pop_macro("warn")
#undef RAMULATOR_HAD_WARN_MACRO
#endif

#ifdef RAMULATOR_HAD_ERROR_MACRO
#pragma pop_macro("error")
#undef RAMULATOR_HAD_ERROR_MACRO
#endif

// Simple fmt replacement functions
namespace fmt {
    inline std::string format(const std::string& format_str) {
        return format_str;
    }

    // Helper function to convert value to string
    template<typename T>
    std::string to_string_helper(const T& value) {
        std::ostringstream oss;
        oss << value;
        return oss.str();
    }

    // Format function with one argument
    template<typename T1>
    std::string format(const std::string& format_str, const T1& arg1) {
        std::string result = format_str;
        size_t pos = result.find("{}");
        if (pos != std::string::npos) {
            result.replace(pos, 2, to_string_helper(arg1));
        }
        return result;
    }

    // Format function with two arguments
    template<typename T1, typename T2>
    std::string format(const std::string& format_str, const T1& arg1, const T2& arg2) {
        std::string result = format_str;
        size_t pos = result.find("{}");
        if (pos != std::string::npos) {
            result.replace(pos, 2, to_string_helper(arg1));
            pos = result.find("{}");
            if (pos != std::string::npos) {
                result.replace(pos, 2, to_string_helper(arg2));
            }
        }
        return result;
    }

    // Format function with three arguments (for the DDR4 density error)
    template<typename T1, typename T2, typename T3>
    std::string format(const std::string& format_str, const T1& arg1, const T2& arg2, const T3& arg3) {
        std::string result = format_str;
        size_t pos = result.find("{}");
        if (pos != std::string::npos) {
            result.replace(pos, 2, to_string_helper(arg1));
            pos = result.find("{}");
            if (pos != std::string::npos) {
                result.replace(pos, 2, to_string_helper(arg2));
                pos = result.find("{}");
                if (pos != std::string::npos) {
                    result.replace(pos, 2, to_string_helper(arg3));
                }
            }
        }
        return result;
    }

    // Format function with eight arguments for detailed debug output
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
    std::string format(const std::string& format_str, const T1& arg1, const T2& arg2, const T3& arg3, const T4& arg4, const T5& arg5, const T6& arg6, const T7& arg7, const T8& arg8) {
        std::string result = format_str;
        size_t pos = result.find("{}");
        if (pos != std::string::npos) {
            result.replace(pos, 2, to_string_helper(arg1));
            pos = result.find("{}");
            if (pos != std::string::npos) {
                result.replace(pos, 2, to_string_helper(arg2));
                pos = result.find("{}");
                if (pos != std::string::npos) {
                    result.replace(pos, 2, to_string_helper(arg3));
                    pos = result.find("{}");
                    if (pos != std::string::npos) {
                        result.replace(pos, 2, to_string_helper(arg4));
                        pos = result.find("{}");
                        if (pos != std::string::npos) {
                            result.replace(pos, 2, to_string_helper(arg5));
                            pos = result.find("{}");
                            if (pos != std::string::npos) {
                                result.replace(pos, 2, to_string_helper(arg6));
                                pos = result.find("{}");
                                if (pos != std::string::npos) {
                                    result.replace(pos, 2, to_string_helper(arg7));
                                    pos = result.find("{}");
                                    if (pos != std::string::npos) {
                                        result.replace(pos, 2, to_string_helper(arg8));
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        return result;
    }

    template<typename Container>
    std::string join(const Container& container, const std::string& separator) {
        if (container.empty()) {
            return "";
        }

        std::ostringstream oss;
        typename Container::const_iterator it = container.begin();
        oss << *it;

        for (++it; it != container.end(); ++it) {
            oss << separator << *it;
        }

        return oss.str();
    }

    template<typename... Args>
    using format_string = const char*;
}

#endif // RAMULATOR_SIMPLE_LOGGING_H