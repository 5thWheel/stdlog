#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>
#include <mutex>
#include <memory>
#include <filesystem>
#include <print>
#include <format>

namespace fs = std::filesystem;

enum class LogLevel {
    NONE,
    TRACE,
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CRITICAL,
    ALL
};

class RollingLogger {
public:
    struct Config {
        std::string log_directory = "./logs";
        std::string filename_prefix = "app";
        size_t max_file_size = 10 * 1024 * 1024; // 10 MB
        std::chrono::hours roll_time_interval{24}; // 24 hours
        LogLevel min_level = LogLevel::DEBUG;
    };

private:
    Config config;
    std::ofstream file;
    std::mutex write_mutex;
    std::chrono::system_clock::time_point next_roll_time;
    std::string current_filename;
    size_t current_file_size = 0;

    static constexpr std::string_view level_strings[] = {
        "NONE", "TRACE", "DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL", "ALL"
    };

    std::string get_timestamp() const {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S")
           << '.' << std::setfill('0') << std::setw(3) << ms.count();
        return ss.str();
    }

    std::string get_filename_with_timestamp() const {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        
        std::stringstream ss;
        ss << config.log_directory << '/'
           << config.filename_prefix << '_'
           << std::put_time(std::localtime(&time), "%Y%m%d_%H%M%S")
           << ".log";
        return ss.str();
    }

    void open_new_file() {
        if (file.is_open()) {
            file.close();
        }

        if (!fs::exists(config.log_directory)) {
            fs::create_directories(config.log_directory);
        }

        current_filename = get_filename_with_timestamp();
        file.open(current_filename, std::ios::app);
        
        if (!file.is_open()) {
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
        if (!file.is_open() || should_roll_by_size() || should_roll_by_time()) {
            open_new_file();
        }
    }

    void write_to_file(std::string_view message) {
        if (!file.is_open()) {
            open_new_file();
        }

        check_and_roll();

        file << message << '\n';
        file.flush();
        current_file_size += message.length() + 1; // +1 for newline
    }

public:
    explicit RollingLogger(const Config& cfg = Config()) : config(cfg) {
        open_new_file();
    }

    ~RollingLogger() {
        if (file.is_open()) {
            file.close();
        }
    }

    // Delete copy operations
    RollingLogger(const RollingLogger&) = delete;
    RollingLogger& operator=(const RollingLogger&) = delete;

    // Allow move operations
    RollingLogger(RollingLogger&&) noexcept = default;
    RollingLogger& operator=(RollingLogger&&) noexcept = default;

    template<typename... Args>
    void log(LogLevel level, std::format_string<Args...> fmt, Args&&... args) {
        if (level < config.min_level) {
            return;
        }

        std::lock_guard<std::mutex> lock(write_mutex);

        std::string message = std::format(
            "[{}] [{}] {}",
            get_timestamp(),
            level_strings[static_cast<int>(level)],
            std::format(fmt, std::forward<Args>(args)...)
        );

        // Print to console
        std::println("{}", message);

        // Write to file
        write_to_file(message);
    }

    template<typename... Args>
    void debug(std::format_string<Args...> fmt, Args&&... args) {
        log(LogLevel::DEBUG, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void info(std::format_string<Args...> fmt, Args&&... args) {
        log(LogLevel::INFO, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void warning(std::format_string<Args...> fmt, Args&&... args) {
        log(LogLevel::WARNING, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void error(std::format_string<Args...> fmt, Args&&... args) {
        log(LogLevel::ERROR, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void critical(std::format_string<Args...> fmt, Args&&... args) {
        log(LogLevel::CRITICAL, fmt, std::forward<Args>(args)...);
    }

    void set_min_level(LogLevel level) {
        std::lock_guard<std::mutex> lock(write_mutex);
        config.min_level = level;
    }

    std::string get_current_log_file() const {
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
