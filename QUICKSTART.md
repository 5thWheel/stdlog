# Quick Start Guide

Get the C++23 Rolling Logger up and running in minutes!

## 1. Minimal Example

Create a file `main.cpp`:

```cpp
#include "rolling_logger.hpp"

int main() {
    RollingLogger logger;
    
    logger.info("Hello, World!");
    logger.debug("Debug message");
    logger.error("Error message");
    
    return 0;
}
```

Compile with:
```bash
g++ -std=c++23 -o app main.cpp -lstdc++exp
```

## 2. Simple Configuration

```cpp
#include "rolling_logger.hpp"

int main() {
    RollingLogger::Config config;
    config.log_directory = "./my_logs";
    config.filename_prefix = "myapp";
    config.max_file_size = 5 * 1024 * 1024;  // 5 MB
    
    RollingLogger logger(config);
    
    logger.info("Application started");
    logger.info("User {} logged in", "alice");
    
    return 0;
}
```

## 3. Log Levels

```cpp
logger.debug("Debug info");      // Detailed diagnostic info
logger.info("Info message");     // General information
logger.warning("Warning!");      // Something unexpected
logger.error("An error");        // Error condition
logger.critical("Critical!");    // Severe error
```

## 4. String Formatting

Use `{}` placeholders just like `std::format`:

```cpp
int id = 123;
std::string name = "Alice";
double value = 42.5;

logger.info("ID: {}, Name: {}, Value: {:.1f}", id, name, value);
// Output: [2024-01-15 14:30:22.567] [INFO] ID: 123, Name: Alice, Value: 42.5
```

## 5. Runtime Configuration

Change settings on the fly:

```cpp
// Change minimum log level
logger.set_min_level(LogLevel::WARNING);

// Update rolling parameters
logger.set_max_file_size(10 * 1024 * 1024);
logger.set_roll_time_interval(std::chrono::hours(24));
```

## 6. Get Current Log File

```cpp
std::string logfile = logger.get_current_log_file();
std::println("Logging to: {}", logfile);
```

## 7. Thread-Safe Logging

Safely log from multiple threads:

```cpp
std::thread t1([&logger]() {
    logger.info("Message from thread 1");
});

std::thread t2([&logger]() {
    logger.info("Message from thread 2");
});

t1.join();
t2.join();
```

## 8. Exception Logging

```cpp
try {
    // some operation
    throw std::runtime_error("Something went wrong");
} catch (const std::exception& e) {
    logger.error("Exception: {}", e.what());
}
```

## Configuration Defaults

| Setting | Default |
|---------|---------|
| Log Directory | `./logs` |
| Filename Prefix | `app` |
| Max File Size | 10 MB |
| Roll Interval | 24 hours |
| Min Log Level | DEBUG |

## Compilation Commands

### GCC (13+)
```bash
g++ -std=c++23 -o app main.cpp -lstdc++exp
```

### Clang (16+)
```bash
clang++ -std=c++2c -o app main.cpp
```

### MSVC (2022+)
```bash
cl /std:c++latest main.cpp
```

### With CMake
```bash
mkdir build && cd build
cmake ..
cmake --build .
./logger_example
```

## Log File Structure

Files are automatically created in the configured directory with timestamps:

```
logs/
├── app_20240115_143022.log
├── app_20240115_150500.log
├── app_20240116_000000.log
└── app_20240116_120530.log
```

Each file rolls when:
1. **Size Limit**: File reaches `max_file_size`
2. **Time Limit**: `roll_time_interval` has elapsed

## Log Format

```
[TIMESTAMP] [LEVEL] MESSAGE

Examples:
[2024-01-15 14:30:22.567] [INFO] User alice logged in
[2024-01-15 14:30:23.123] [DEBUG] Processing value: 3.14
[2024-01-15 14:30:24.456] [ERROR] Connection timeout
```

## Output Destinations

Logs are written to **both**:
1. **Console** (stdout/stderr) - via `std::println`
2. **File** - in the configured log directory

## Tips & Tricks

### Performance
For high-frequency logging, batch your messages:
```cpp
std::stringstream ss;
for (int i = 0; i < 1000; ++i) {
    ss << "item " << i << ", ";
}
logger.debug("{}", ss.str());
```

### Debug-Only Logging
Compile with preprocessor flags:
```cpp
#ifdef DEBUG_LOGGING
    logger.debug("Debug info");
#endif
```

### Structured Logging
Create formatted structures:
```cpp
struct LogEntry {
    std::string user;
    int action_id;
    double duration;
};

auto entry = LogEntry{"alice", 42, 1.5};
logger.info("User: {}, Action: {}, Duration: {} ms", 
    entry.user, entry.action_id, entry.duration);
```

## Common Issues

**Issue**: "Header file not found"
- **Solution**: Make sure `rolling_logger.hpp` is in the same directory or add `-I` flag: `g++ -I/path/to/header`

**Issue**: "undefined reference to `std::println`"
- **Solution**: Use C++23 flag: `-std=c++23` for GCC/Clang, `/std:c++latest` for MSVC

**Issue**: Permission denied creating log files
- **Solution**: Ensure the log directory path is writable

**Issue**: Files not rolling based on time
- **Solution**: Check that your system time is set correctly; rolling is based on `std::chrono::system_clock`

## Next Steps

1. Check `README.md` for comprehensive documentation
2. Review `example_usage.cpp` for more examples
3. Try `advanced_example.cpp` for multi-threading and advanced features
4. Integrate into your project!

## Support

For detailed documentation, see the `README.md` file included in the package.
