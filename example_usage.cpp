#include "rolling_logger.hpp"
#include <thread>
#include <chrono>

int main() {
    // Create a logger with custom configuration
    RollingLogger::Config config;
    config.log_directory = "./logs";
    config.filename_prefix = "application";
    config.max_file_size = 5 * 1024 * 1024; // 5 MB for file rolling
    config.roll_time_interval = std::chrono::hours(24); // Daily rotation
    config.min_level = LogLevel::DEBUG;

    RollingLogger logger(config);

    std::println("Logger initialized. Logging to: {}\n", logger.get_current_log_file());

    // Log messages at different levels
    logger.debug("This is a debug message");
    logger.info("Application started successfully");
    logger.warning("This is a warning message");
    logger.error("An error occurred during processing");
    logger.critical("Critical system failure!");

    // Log with formatted arguments
    int user_id = 42;
    std::string username = "john_doe";
    double value = 3.14159;

    logger.info("User {} (ID: {}) logged in", username, user_id);
    logger.debug("Processing value: {:.2f}", value);

    // Simulate some work with logging
    for (int i = 1; i <= 5; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        logger.info("Processing iteration {} of 5", i);
    }

    // Log a more complex scenario
    try {
        throw std::runtime_error("Example exception");
    } catch (const std::exception& e) {
        logger.error("Exception caught: {}", e.what());
    }

    // Change logging level at runtime
    logger.set_min_level(LogLevel::WARNING);
    logger.debug("This debug won't be logged (level changed)");
    logger.warning("But warnings will still be logged");

    // Reset to debug level
    logger.set_min_level(LogLevel::DEBUG);

    // Demonstrate configurable rolling
    logger.set_max_file_size(2 * 1024 * 1024); // Change to 2 MB
    logger.set_roll_time_interval(std::chrono::hours(12)); // Change to 12 hours

    logger.info("Logger configuration updated");
    logger.info("Current log file: {}", logger.get_current_log_file());

    // Log some entries that might trigger rolling
    for (int i = 0; i < 10; ++i) {
        logger.info("Large message #{}: {}", i, 
            std::string(100, 'X')); // Log repeated characters to increase file size
    }

    logger.info("Application shutting down");

    return 0;
}
