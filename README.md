# C++23 Rolling Logger

A modern, header-only logging library for C++23 that supports rolling files based on both size and time intervals. Uses `std::println` for formatted output.

## Features

- **std::println Integration**: Leverages C++23's `std::println` for modern, formatted output
- **Dual Rolling Strategy**:
  - **Size-based Rolling**: Automatically creates a new log file when the current one reaches a configurable size limit
  - **Time-based Rolling**: Creates a new log file at regular intervals (e.g., daily rotation)
- **Thread-Safe**: Uses `std::mutex` to ensure thread-safe logging operations
- **Log Levels**: Supports 5 log levels (DEBUG, INFO, WARNING, ERROR, CRITICAL)
- **Formatted Logging**: Uses `std::format` for type-safe string formatting
- **Console & File Output**: Logs to both console and file simultaneously
- **Runtime Configuration**: Change logging levels, file sizes, and roll intervals at runtime
- **Header-Only**: Easy integration with just an include

## Requirements

- C++23 compatible compiler:
  - GCC 13+
  - Clang 16+
  - MSVC 2022+
- CMake 3.23+ (for building examples)

## Installation

Simply include `rolling_logger.hpp` in your project:

```cpp
#include "rolling_logger.hpp"
```

## Usage

### Basic Example

```cpp
#include "rolling_logger.hpp"

int main() {
    RollingLogger logger;
    
    logger.debug("Debug message");
    logger.info("Info message");
    logger.warning("Warning message");
    logger.error("Error message");
    logger.critical("Critical message");
    
    return 0;
}
```

### Custom Configuration

```cpp
RollingLogger::Config config;
config.log_directory = "./logs";
config.filename_prefix = "myapp";
config.max_file_size = 10 * 1024 * 1024;  // 10 MB
config.roll_time_interval = std::chrono::hours(24);
config.min_level = LogLevel::DEBUG;

RollingLogger logger(config);
```

### Formatted Logging

The logger uses `std::format` for type-safe formatting:

```cpp
int user_id = 42;
std::string username = "alice";
double value = 3.14;

logger.info("User {} (ID: {}) value: {:.2f}", username, user_id, value);
```

### Runtime Configuration

```cpp
// Change minimum log level
logger.set_min_level(LogLevel::WARNING);

// Update file size limit
logger.set_max_file_size(5 * 1024 * 1024);  // 5 MB

// Update rolling interval
logger.set_roll_time_interval(std::chrono::hours(12));
```

### Querying Current State

```cpp
std::string current_file = logger.get_current_log_file();
std::println("Currently logging to: {}", current_file);
```

## Configuration Options

### RollingLogger::Config

| Parameter | Default | Description |
|-----------|---------|-------------|
| `log_directory` | `"./logs"` | Directory where log files are stored |
| `filename_prefix` | `"app"` | Prefix for log filenames |
| `max_file_size` | `10 * 1024 * 1024` (10 MB) | Maximum size before file rolling |
| `roll_time_interval` | `24 hours` | Time interval for rolling files |
| `min_level` | `LogLevel::DEBUG` | Minimum log level to output |

## Log Levels

```cpp
enum class LogLevel {
    DEBUG,      // Detailed diagnostic information
    INFO,       // Informational messages
    WARNING,    // Warning messages
    ERROR,      // Error messages
    CRITICAL    // Critical/severe error messages
};
```

## File Naming

Log files are automatically named with timestamps:

```
logs/app_20240115_143022.log
logs/app_20240115_150500.log
logs/app_20240116_000000.log
```

This naming scheme ensures:
- Files are ordered chronologically
- Old files are easily identified and can be archived
- Multiple log files don't overwrite each other

## Log Format

Each log entry includes:
- Timestamp with millisecond precision
- Log level
- Formatted message

Example output:
```
[2024-01-15 14:30:22.567] [INFO] User alice (ID: 42) logged in
[2024-01-15 14:30:23.123] [DEBUG] Processing value: 3.14
[2024-01-15 14:30:24.456] [WARNING] Low memory condition detected
```

## Thread Safety

The logger is thread-safe for concurrent logging from multiple threads. All write operations are protected by a `std::mutex`:

```cpp
std::thread t1([&logger]() {
    for (int i = 0; i < 100; ++i) {
        logger.info("Thread 1: iteration {}", i);
    }
});

std::thread t2([&logger]() {
    for (int i = 0; i < 100; ++i) {
        logger.info("Thread 2: iteration {}", i);
    }
});

t1.join();
t2.join();
```

## Building the Example

### Using CMake

```bash
mkdir build
cd build
cmake ..
cmake --build .
./logger_example
```

### Manual Compilation (GCC)

```bash
g++ -std=c++23 -o logger_example example_usage.cpp -lstdc++exp
```

### Manual Compilation (Clang)

```bash
clang++ -std=c++2c -o logger_example example_usage.cpp
```

### Manual Compilation (MSVC)

```bash
cl /std:c++latest example_usage.cpp
```

## Performance Considerations

1. **Lock Contention**: Each log call acquires a mutex. For extremely high-frequency logging, consider batching messages.

2. **File I/O**: Files are flushed after each write. If performance is critical, consider buffering.

3. **String Formatting**: The logger uses `std::format`, which is efficient but has some overhead. Avoid excessive logging in tight loops.

4. **File Rolling**: Size checks are performed on every write, but the actual file rolling is lazy (only performed when necessary).

## Advanced Usage

### Custom Logging with Variadic Templates

```cpp
template<typename... Args>
void log_custom(std::format_string<Args...> fmt, Args&&... args) {
    logger.info(fmt, std::forward<Args>(args)...);
}
```

### Exception Logging

```cpp
try {
    // some operation
} catch (const std::exception& e) {
    logger.error("Exception: {}", e.what());
}
```

### Performance Monitoring

```cpp
auto start = std::chrono::high_resolution_clock::now();
// perform operation
auto end = std::chrono::high_resolution_clock::now();
auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
logger.debug("Operation took {} ms", duration.count());
```

## Limitations

1. **Single Logger Instance**: The current implementation is designed for single logger usage. For multiple independent loggers, create separate instances.

2. **Directory Creation**: The logger automatically creates the log directory if it doesn't exist, but parent directories must exist.

3. **No Network Logging**: This logger writes only to local files and stdout.

4. **No Filtering**: All threads use the same log level filtering.

## Future Enhancements

Potential improvements for future versions:
- Asynchronous logging with a background thread
- Multiple output destinations (syslog, network)
- Pattern-based filtering
- Compressed archive support for old logs
- Structured logging (JSON format)
- Log rotation policies (keep N files, delete older than X days)

## License

This logger is provided as-is for educational and production use.

## Compiler Support

| Compiler | Version | Status |
|----------|---------|--------|
| GCC | 13+ | ✅ Supported |
| Clang | 16+ | ✅ Supported |
| MSVC | 2022+ | ✅ Supported |
| Apple Clang | 15+ | ✅ Supported |
