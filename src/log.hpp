#pragma once

#include <format>
#include <iostream>
#include <string>
#include <string_view>

#include <unistd.h>

namespace otus {

class Log {
  public:
    enum Severity { ERROR, WARN, INFO, DEBUG };

    static Log &Get() {
        static Log instance;
        return instance;
    }

    void SetSeverity(Severity level) { level_ = level; }

    template <typename... Args>
    static std::string UnfoldFormat(std::string_view fmt, Args &&...args) {
        return std::vformat(fmt, std::make_format_args(args...));
    }

    template <typename... Args>
    void Debug(std::string_view fmt, Args &&...args) noexcept {
        if (level_ < Severity::DEBUG)
            return;

        std::cout << "[DEBUG]" << UnfoldFormat(fmt, args...) << "\n";
    }

    template <typename... Args>
    void Info(std::string_view fmt, Args &&...args) noexcept {
        if (level_ < Severity::INFO)
            return;

        std::cout << "[INFO]" << UnfoldFormat(fmt, args...) << "\n";
    }

    template <typename... Args>
    void Warn(std::string_view fmt, Args &&...args) noexcept {
        if (level_ < Severity::WARN)
            return;

        std::cout << "[WARN]" << UnfoldFormat(fmt, args...) << "\n";
    }

    template <typename... Args>
    void Error(std::string_view fmt, Args &&...args) noexcept {
        std::cout << "[ERROR]" << UnfoldFormat(fmt, args...) << "\n";
    }

    bool SetSeverityFromArgs(int argc, char *const argv[]) {
        int opt, parsed;
        while ((opt = getopt(argc, argv, "d:")) != -1) {
            switch (opt) {
            case 'd':
                parsed = std::atoi(optarg);
                break;

            default:
                break;
            }
        }

        if (parsed <= otus::Log::Severity::DEBUG &&
            parsed >= otus::Log::Severity::ERROR) {
            level_ = static_cast<otus::Log::Severity>(parsed);
            return true;
        }

        return false;
    }

  private:
    Severity level_ = Severity::WARN;

    Log() = default;
    ~Log() = default;

    Log(const Log &root) = delete;
    Log &operator=(const Log &) = delete;
    Log(Log &&root) = delete;
    Log &operator=(Log &&) = delete;
};

template <typename T>
constexpr auto
get_type_name() -> std::string_view {
#if defined(__clang__)
    constexpr auto prefix = std::string_view{"[T = "};
    constexpr auto suffix = "]";
    constexpr auto function = std::string_view{__PRETTY_FUNCTION__};
#elif defined(__GNUC__)
    constexpr auto prefix = std::string_view{"with T = "};
    constexpr auto suffix = "; ";
    constexpr auto function = std::string_view{__PRETTY_FUNCTION__};
#elif defined(_MSC_VER)
    constexpr auto prefix = std::string_view{"get_type_name<"};
    constexpr auto suffix = ">(void)";
    constexpr auto function = std::string_view{__FUNCSIG__};
#else
#error Unsupported compiler
#endif

    const auto start = function.find(prefix) + prefix.size();
    const auto end = function.find(suffix);
    const auto size = end - start;

    return function.substr(start, size);
}

}   // namespace otus
