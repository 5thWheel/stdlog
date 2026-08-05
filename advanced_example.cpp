#include "stdlog/logger.hpp"
#include <thread>
#include <random>

using stdlog::Logger;
using stdlog::LogLevel;

void simulate_worker(Logger& logger, int worker_id, int iterations) {
    std::random_device rd;
    std::mt19937 gen(rd() + worker_id);
    std::uniform_int_distribution<> dis(100, 500);

    for (int i = 0; i < iterations; ++i) {
        auto delay = dis(gen);
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));

        switch (i % 4) {
            case 0:
                logger.debug("Worker {}: Debug message iteration {}", worker_id, i);
                break;
            case 1:
                logger.info("Worker {}: Processing data batch {} completed", worker_id, i);
                break;
            case 2:
                logger.warning("Worker {}: Memory usage at {}%", worker_id, 70 + (i % 20));
                break;
            case 3:
                logger.error("Worker {}: Failed to process item {}", worker_id, i);
                break;
        }
    }

    logger.info("Worker {} finished (processed {} items)", worker_id, iterations);
}

void demonstrate_file_rolling(Logger& logger) {
    logger.info("=== Demonstrating File Rolling ===");
    
    // Log messages to trigger size-based rolling
    std::string large_message(1000, 'X'); // 1KB message
    
    for (int i = 0; i < 100; ++i) {
        logger.info("Message {}: {}", i, large_message);
        
        if (i % 25 == 0) {
            std::println("Current log file: {}", logger.get_current_log_file());
        }
    }
}

void demonstrate_exception_handling(Logger& logger) {
    logger.info("=== Demonstrating Exception Handling ===");
    
    try {
        std::vector<int> data = {1, 2, 3};
        logger.info("Accessing index 5 in vector of size 3");
        int i = data.at(5); // This will throw
    } catch (const std::out_of_range& e) {
        logger.error("Exception caught: {}", e.what());
    }

    try {
        throw std::runtime_error("Simulated critical error");
    } catch (const std::exception& e) {
        logger.critical("Critical exception: {}", e.what());
    }
}

void demonstrate_performance_logging(Logger& logger) {
    logger.info("=== Demonstrating Performance Logging ===");
    
    // Simulate various operations and log their performance
    std::vector<std::pair<std::string, int>> operations = {
        {"database_query", 150},
        {"file_write", 45},
        {"network_request", 250},
        {"data_processing", 80}
    };

    for (const auto& [op_name, duration_ms] : operations) {
        if (duration_ms > 200) {
            logger.warning("Slow operation '{}' took {} ms", op_name, duration_ms);
        } else if (duration_ms > 100) {
            logger.info("Operation '{}' took {} ms", op_name, duration_ms);
        } else {
            logger.debug("Fast operation '{}' took {} ms", op_name, duration_ms);
        }
    }
}

void demonstrate_level_filtering(Logger& logger) {
    logger.info("=== Demonstrating Log Level Filtering ===");
    
    // Log at all levels
    logger.debug("This is a debug message (visible)");
    logger.info("This is an info message (visible)");
    
    // Change to WARNING level
    logger.set_min_level(LogLevel::WARNING);
    logger.info("This info message will NOT be logged (level changed to WARNING)");
    logger.warning("This warning message is visible");
    logger.error("This error message is visible");
    
    // Reset to DEBUG
    logger.set_min_level(LogLevel::DEBUG);
    logger.info("This info message is visible again (reset to DEBUG)");
}

int main() {
    // Configure logger with smaller file size for demonstration
    Logger::Config config;
    config.log_directory = "./logs_advanced";
    config.filename_prefix = "advanced_demo";
    config.max_file_size = 1024 * 1024; // 1 MB for easier testing
    config.roll_time_interval = std::chrono::hours(1);
    config.min_level = LogLevel::DEBUG;

    Logger logger(config);

    std::println("╔════════════════════════════════════════════════════════╗");
    std::println("║     C++23 Rolling Logger - Advanced Example             ║");
    std::println("╚════════════════════════════════════════════════════════╝\n");

    logger.info("Application started");
    logger.info("Log directory: {}", config.log_directory);
    logger.info("Current log file: {}", logger.get_current_log_file());

    // Test 1: Basic level testing
    std::println("\n[TEST 1] Testing log levels...\n");
    logger.debug("Debug level message");
    logger.info("Info level message");
    logger.warning("Warning level message");
    logger.error("Error level message");
    logger.critical("Critical level message");

    // Test 2: Formatted output
    std::println("\n[TEST 2] Testing formatted output...\n");
    int count = 42;
    double pi = 3.14159265;
    std::string app_name = "MyApp";
    logger.info("Application: {}, Version: {}, Pi: {:.4f}", app_name, count, pi);

    // Test 3: Level filtering
    std::println("\n[TEST 3] Testing level filtering...\n");
    demonstrate_level_filtering(logger);

    // Test 4: Exception handling
    std::println("\n[TEST 4] Testing exception handling...\n");
    demonstrate_exception_handling(logger);

    // Test 5: Performance logging
    std::println("\n[TEST 5] Testing performance logging...\n");
    demonstrate_performance_logging(logger);

    // Test 6: File rolling
    std::println("\n[TEST 6] Testing file rolling (may take a moment)...\n");
    demonstrate_file_rolling(logger);

    // Test 7: Multi-threaded logging
    std::println("\n[TEST 7] Testing multi-threaded logging...\n");
    logger.info("Starting multi-threaded test with 4 workers");
    
    std::vector<std::thread> threads;
    const int num_workers = 4;
    const int iterations_per_worker = 10;

    for (int i = 0; i < num_workers; ++i) {
        threads.emplace_back(simulate_worker, std::ref(logger), i, iterations_per_worker);
    }

    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }

    logger.info("All worker threads completed");

    // Final summary
    std::println("\n[SUMMARY]\n");
    logger.info("Advanced example completed successfully");
    logger.info("Check log files in: {}", config.log_directory);
    logger.info("Application shutting down");

    std::println("\n╔════════════════════════════════════════════════════════╗");
    std::println("║                Example completed!                       ║");
    std::println("║         Check {} for all log files          ║", config.log_directory);
    std::println("╚════════════════════════════════════════════════════════╝\n");

    return 0;
}
