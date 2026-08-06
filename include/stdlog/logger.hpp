#pragma once

//#include <iostream>
//#include <fstream>
#include <cstdio>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>
#include <mutex>
#include <memory>
#include <filesystem>
#include <print>
#include <format>
#include <source_location>

namespace fs = std::filesystem;

#define STDLOG_VERSION_MAJOR 0
#define STDLOG_VERSION_MINOR 10
#define STDLOG_VERSION_PATCH 0

#ifndef STDLOG_BEGIN_NAMESPACE
#define STDLOG_BEGIN_NAMESPACE \
namespace stdlog {  \
inline namespace v0 { \

#endif

#ifndef STDLOG_END_NAMESPACE
#define STDLOG_END_NAMESPACE \
    } \
}
#endif


STDLOG_BEGIN_NAMESPACE

enum class LogLevel : uint16_t {
    NONE,
    TRACE,
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CRITICAL,
    ALL
};

class Logger final {
public:
    struct Config {
        std::string log_directory = "./logs";
        std::string filename_prefix = "app";
        std::string filename_extension = ".log";
        size_t max_file_size = 10 * 1024 * 1024; // 10 MB
        std::chrono::hours roll_time_interval{ 24 }; // 24 hours
        LogLevel min_level = LogLevel::INFO;

        friend class Logger;

    private:
        std::string get_filename_with_timestamp() const {
            auto now = std::chrono::system_clock::now();
            auto tp = std::chrono::time_point_cast<std::chrono::seconds>(now);
            return std::format("{}/{}_{:%Y%m%d_%H%M%S}{}",
                log_directory, filename_prefix, tp, filename_extension);
        }
    };

private:
    Config config;
    FILE* file = NULL;
    std::mutex write_mutex{};
    std::chrono::system_clock::time_point next_roll_time{};
    std::string current_filename{};
    size_t current_file_size = 0;

    static constexpr std::string_view level_string(stdlog::LogLevel level) {
        switch (level) {
        case stdlog::LogLevel::NONE: return "NONE";
        case stdlog::LogLevel::TRACE: return "TRACE";
        case stdlog::LogLevel::DEBUG: return "DEBUG";
        case stdlog::LogLevel::INFO: return "INFO";
        case stdlog::LogLevel::WARNING: return "WARNING";
        case stdlog::LogLevel::ERROR: return "ERROR";
        case stdlog::LogLevel::CRITICAL: return "CRITICAL";
        case stdlog::LogLevel::ALL: return "ALL";
        default: return "";
        }
    }

    static constexpr std::string_view level_color(stdlog::LogLevel level) {
        switch (level) {
        case stdlog::LogLevel::INFO: return "\033[32m";
        case stdlog::LogLevel::WARNING: return "\033[33m";
        case stdlog::LogLevel::ERROR: return "\033[31m";
        case stdlog::LogLevel::CRITICAL: return "\033[91m";
        default: return "";
        }
    }

    std::string source_to_string(const std::source_location& source) {
        return std::format("{}:{}:{}",
            fs::path(source.file_name()).filename().string(),
            source.function_name(),
            source.line()
        );
    }

    void close_file() {
        if (file) {
            fclose(file);
        }

        file = NULL;
    }

    void open_new_file() {
        close_file();

        if (!fs::exists(config.log_directory)) {
            fs::create_directories(config.log_directory);
        }

        current_filename = config.get_filename_with_timestamp();
        file = std::fopen(current_filename.c_str(), "w");

        if (file == NULL) {
            std::println(stderr, "Failed to open log file: {}", current_filename);
        }

        current_file_size = 0;
        next_roll_time = std::chrono::system_clock::now() + config.roll_time_interval;
    }

    bool should_roll_by_size() const {
        return current_file_size >= config.max_file_size;
    }

    bool should_roll_by_time() const {
        return std::chrono::system_clock::now() >= next_roll_time;
    }

    void check_and_roll() {
        if (!file || should_roll_by_size() || should_roll_by_time()) {
            open_new_file();
        }
    }

    void write_to_file(std::string_view message) {
        check_and_roll();

        std::println(file, "{}", message);

        current_file_size += message.length() + 1; // +1 for newline
    }

public:
    explicit Logger(const Config& cfg = Config()) : config(cfg) {
        open_new_file();
    }

    ~Logger() {
        if (file) {
            fclose(file);
        }
    }

    // Delete copy operations
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    // Allow move operations
    Logger(Logger&&) noexcept = default;
    Logger& operator=(Logger&&) noexcept = default;

    template<typename... Args>
    //void log(LogLevel level, const std::source_location& source, std::format_string<Args...> fmt, Args&&... args) {
    void log(LogLevel level, const char* source_file, const char* source_function, const size_t source_line_no, std::format_string<Args...> fmt, Args&&... args) {
        if (level < config.min_level) {
            return;
        }

        std::lock_guard<std::mutex> lock(write_mutex);

        auto now = std::chrono::system_clock::now();

        auto tp = std::chrono::time_point_cast<std::chrono::seconds>(now);

        std::string message = std::format("{:%F %T} [{}:{}:{}] [{}] {}", tp,
            /*source_to_string(source),*/ fs::path(source_file).filename().string(), source_function, source_line_no,
            level_string(level),
            std::format(fmt, std::forward<Args>(args)...)
        );

#ifdef _DEBUG
        // Print to console

        std::println("{}{}\033[0m", level_color(level), message);
#endif

        // Write to file
        write_to_file(message);
    }

    //template<typename... Args>
    //struct debug {
    //    debug(std::format_string<Args...> fmt, Args&&... args, std::source_location source = std::source_location::current()) {
    //        log(LogLevel::DEBUG, source, fmt, std::forward<Args>(args)...);
    //    }
    // 
    //    debug(std::format_string<Args...> fmt, std::source_location source = std::source_location::current()) {
    //        log(LogLevel::DEBUG, source, fmt);
    //    }
    //};
    // 
    //template<typename... Args>
    //debug(std::format_string<Args...> fmt, Args&&... args) -> debug<Args...>;
    // 
    //template<typename... Args>
    //debug(std::format_string<Args...> fmt) -> debug<Args...>;

    //template<typename... Args>
    //void debug(const std::source_location& source, std::format_string<Args...> fmt, Args&&... args) {
    //    log(LogLevel::DEBUG, source, fmt, std::forward<Args>(args)...);
    //}

    //template<typename... Args>
    //void info(const std::source_location& source, std::format_string<Args...> fmt, Args&&... args) {
    //    log(LogLevel::INFO, source, fmt, std::forward<Args>(args)...);
    //}

    //template<typename... Args>
    //void warning(const std::source_location& source, std::format_string<Args...> fmt, Args&&... args) {
    //    log(LogLevel::WARNING, source, fmt, std::forward<Args>(args)...);
    //}

    //template<typename... Args>
    //void error(const std::source_location& source, std::format_string<Args...> fmt, Args&&... args) {
    //    log(LogLevel::ERROR, source, fmt, std::forward<Args>(args)...);
    //}

    //template<typename... Args>
    //void critical(const std::source_location& source, std::format_string<Args...> fmt, Args&&... args) {
    //    log(LogLevel::CRITICAL, source, fmt, std::forward<Args>(args)...);
    //}

    void set_min_level(LogLevel level) {
        std::lock_guard<std::mutex> lock(write_mutex);
        config.min_level = level;
    }

    const std::string get_current_log_file() const {
        //std::lock_guard<std::mutex> lock(write_mutex);
        return current_filename;
    }

    void set_max_file_size(size_t size) {
        std::lock_guard<std::mutex> lock(write_mutex);
        config.max_file_size = size;
    }

    void set_roll_time_interval(std::chrono::hours interval) {
        std::lock_guard<std::mutex> lock(write_mutex);
        config.roll_time_interval = interval;
    }
};

extern std::unique_ptr<stdlog::Logger> the_logger;

STDLOG_END_NAMESPACE

#define STDLOG_RECORD(LEVEL, fmt, ...)	stdlog::the_logger->log(LEVEL, __FILE__, __func__, __LINE__, fmt __VA_OPT__(,) __VA_ARGS__)

#define log_debug(fmt,...)      STDLOG_RECORD(stdlog::LogLevel::DEBUG, fmt __VA_OPT__(,) __VA_ARGS__)
#define log_info(fmt,...)       STDLOG_RECORD(stdlog::LogLevel::INFO, fmt __VA_OPT__(,) __VA_ARGS__)
#define log_warn(fmt,...)       STDLOG_RECORD(stdlog::LogLevel::WARNING, fmt __VA_OPT__(,) __VA_ARGS__)
#define log_warning(fmt,...)    STDLOG_RECORD(stdlog::LogLevel::WARNING, fmt __VA_OPT__(,) __VA_ARGS__)
#define log_error(fmt,...)      STDLOG_RECORD(stdlog::LogLevel::ERROR, fmt __VA_OPT__(,) __VA_ARGS__)
#define log_fatal(fmt,...)      STDLOG_RECORD(stdlog::LogLevel::CRITICAL, fmt __VA_OPT__(,) __VA_ARGS__)
#define log_critical(fmt,...)   STDLOG_RECORD(stdlog::LogLevel::CRITICAL, fmt __VA_OPT__(,) __VA_ARGS__)
