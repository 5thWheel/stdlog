#include "stdlog/logger.hpp"
#include <thread>

using stdlog::Logger;
using stdlog::LogLevel;

// Declare extern logger variable
std::unique_ptr<stdlog::Logger> stdlog::the_logger = nullptr;

int main() {
    // Create a logger with custom configuration
    Logger::Config config;
    config.log_directory = "./logs";
    config.filename_prefix = "logger_test";
    config.max_file_size = 1024; // 1 KB for file rolling
    config.roll_time_interval = std::chrono::hours(1); // Daily rotation
    config.min_level = LogLevel::DEBUG;

    stdlog::the_logger = std::make_unique<stdlog::Logger>(config);
    Logger& logger = *(stdlog::the_logger.get());

    PRINT_LINE("Logger initialized. Logging to: {}\n", logger.get_current_log_file());

    // Log messages at different levels
    log_debug("This is a debug message");
    log_info("Application started successfully");
    log_warning("This is a warning message");
    log_error("An error occurred during processing");
    log_critical("Critical system failure!");

    // Log with formatted arguments
    int user_id = 42;
    std::string username = "john_doe";
    double value = 3.14159;

    log_info("User {} (ID: {}) logged in", username, user_id);
    log_debug("Processing value: {:.2f}", value);

    // Simulate some work with logging
    for (int i = 1; i <= 5; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        log_info("Processing iteration {} of 5", i);
    }

    // Log a more complex scenario
    try {
        throw std::runtime_error("Example exception");
    } catch (const std::exception& e) {
        log_error("Exception caught: {}", e.what());
    }

    // Change logging level at runtime
    logger.set_min_level(LogLevel::WARNING);
    log_debug("This debug won't be logged (level changed)");
    log_warning("But warnings will still be logged");

    // Reset to debug level
    logger.set_min_level(LogLevel::DEBUG);

    // Demonstrate configurable rolling
    logger.set_max_file_size(2 * 1024 * 1024); // Change to 2 MB
    logger.set_roll_time_interval(std::chrono::hours(12)); // Change to 12 hours

    log_info("Logger configuration updated");
    log_info("Current log file: {}", logger.get_current_log_file());

    // Log some entries that might trigger rolling
    for (int i = 0; i < 10; ++i) {
        log_info("Large message #{}: {}", i,
            std::string(100, 'X')); // Log repeated characters to increase file size
    }

    log_info("Application shutting down");

    return 0;
}
