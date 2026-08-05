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
        size_t max_file_size = 1 * 1024 * 1024; // 1 MB
        std::chrono::hours roll_time_interval{ 24 }; // 24 hours
        LogLevel min_level = LogLevel::INFO;
    };

private:
    Config config;
    //std::ofstream file;
    FILE* file = NULL;
    std::mutex write_mutex{};
    std::chrono::system_clock::time_point next_roll_time{};
    std::string current_filename{};
    size_t current_file_size = 0;

    static constexpr std::string_view level_strings[] = {
        "NONE", "TRACE", "DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL", "ALL"
    };

    //std::string get_timestamp() const {
    //    auto now = std::chrono::system_clock::now();
    //    auto time = std::chrono::system_clock::to_time_t(now);
    //    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
    //        now.time_since_epoch()) % 1000;
    // 
    //    std::stringstream ss;
    //    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S")
    //        << '.' << std::setfill('0') << std::setw(3) << ms.count();
    //    return ss.str();
    //}

    std::string source_to_string(const std::source_location& source) {
        return std::format("{}:{}:{}",
            fs::path(source.file_name()).filename().string(),
            source.function_name(),
            source.line()
        );
    }

    std::string get_filename_with_timestamp() const {
        auto now = std::chrono::system_clock::now();
        auto tp = std::chrono::time_point_cast<std::chrono::seconds>(now);
        return std::format("{}/{}_{:%Y%m%d_%H%M%S}.log",
            config.log_directory, config.filename_prefix, tp);

        //auto time = std::chrono::system_clock::to_time_t(now);
        // 
        //std::stringstream ss;
        //ss << config.log_directory << '/'
        //    << config.filename_prefix << '_'
        //    << std::put_time(std::localtime(&time), "%Y%m%d_%H%M%S")
        //    << ".log";
        //return ss.str();
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

        current_filename = get_filename_with_timestamp();
        //file.open(current_filename, std::ios::app);
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
     
        //file << message << '\n';
        //file.flush();

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
    void log(LogLevel level, const std::source_location& source, std::format_string<Args...> fmt, Args&&... args) {
        if (level < config.min_level) {
            return;
        }

        std::lock_guard<std::mutex> lock(write_mutex);

        auto now = std::chrono::system_clock::now();

        auto tp = std::chrono::time_point_cast<std::chrono::seconds>(now);

        //std::string message = std::format(
        //    "[{}] [{}] {}",
        //    get_timestamp(),
        //    level_strings[static_cast<int>(level)],
        //    std::format(fmt, std::forward<Args>(args)...)
        //);

        std::string message = std::format("{:%F %T} [{}] [{}] {}", tp,
            source_to_string(source),
            level_strings[static_cast<int>(level)],
            std::format(fmt, std::forward<Args>(args)...)
        );

#ifdef _DEBUG
        // Print to console
        std::println("{}", message);
#endif

        // Write to file
        write_to_file(message);
    }

    template<typename... Args>
    void debug(std::format_string<Args...> fmt, Args&&... args) {
        log(LogLevel::DEBUG, std::source_location::current(), fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void info(std::format_string<Args...> fmt, Args&&... args) {
        log(LogLevel::INFO, std::source_location::current(), fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void warning(std::format_string<Args...> fmt, Args&&... args) {
        log(LogLevel::WARNING, std::source_location::current(), fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void error(std::format_string<Args...> fmt, Args&&... args) {
        log(LogLevel::ERROR, std::source_location::current(), fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void critical(std::format_string<Args...> fmt, Args&&... args) {
        log(LogLevel::CRITICAL, std::source_location::current(), fmt, std::forward<Args>(args)...);
    }

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

STDLOG_END_NAMESPACE