# Logger System

A lightweight, thread-safe logging library with configurable output format.

## Design

- **Singleton pattern** — one global `Logger::instance()` shared across all threads
- **Thread-safe** — mutex-protected write operations
- **Compile-time level filtering** — zero overhead for suppressed messages
- **Configurable format** — bitwise flags control which fields appear in output
- **Colored terminal output** — optional ANSI colors per log level

## Quick Start

```cpp
#include <common/logger.h>
using namespace common;

int main() {
    LOG_INFO("Hello world");
    LOG_WARNING("Something suspicious");
    LOG_ERROR("Something broke");

    // Formatted output with variables
    int x = 42;
    LOG_INFO_FMT("The answer is " << x);
}
```

## Log Levels

| Level     | Macro         | Use Case                                 |
| --------- | ------------- | ---------------------------------------- |
| `DEBUG`   | `LOG_DEBUG`   | Development diagnostics, verbose tracing |
| `INFO`    | `LOG_INFO`    | Normal operation milestones              |
| `WARNING` | `LOG_WARNING` | Non-critical issues worth noting         |
| `ERROR`   | `LOG_ERROR`   | Operation failures                       |
| `FATAL`   | `LOG_FATAL`   | Unrecoverable errors                     |

Set minimum level to suppress lower-priority messages:

```cpp
Logger::instance().set_level(LogLevel::WARNING);  // Only WARNING, ERROR, FATAL shown
```

## Format Configuration

### Format Flags

Each field in the log output can be independently toggled:

| Flag                   | Output Example              | Description                   |
| ---------------------- | --------------------------- | ----------------------------- |
| `LogFormat::Timestamp` | `[2026-05-29 14:43:06.826]` | Date + time with milliseconds |
| `LogFormat::Level`     | `[INFO ]`                   | Log level tag                 |
| `LogFormat::ThreadId`  | `[tid:129930682377152]`     | Thread identifier             |
| `LogFormat::Location`  | `[main.cpp:42]`             | Source file and line number   |
| `LogFormat::Message`   | `Hello world`               | The actual message            |

### Presets

| Preset     | Flags           | Output                                                          |
| ---------- | --------------- | --------------------------------------------------------------- |
| `kFull`    | All flags       | `[2026-05-29 14:43:06] [INFO ] [tid:12345] [main.cpp:42] Hello` |
| `kDefault` | No ThreadId     | `[2026-05-29 14:43:06] [INFO ] [main.cpp:42] Hello`             |
| `kCompact` | Level + Message | `[INFO ] Hello`                                                 |

### Usage

```cpp
#include <common/logger.h>
using namespace common;

int main() {
    // Full output (default) — good for debugging multithreaded code
    Logger::instance().set_format(LogFormat::kFull);
    LOG_INFO("All fields visible");
    // [2026-05-29 14:43:06.826] [INFO ] [tid:12345] [main.cpp:10] All fields visible

    // Compact — clean console during learning/testing
    Logger::instance().set_format(LogFormat::kCompact);
    LOG_INFO("Just the essentials");
    // [INFO ] Just the essentials

    // Default — timestamp without thread noise
    Logger::instance().set_format(LogFormat::kDefault);
    LOG_INFO("No thread id");
    // [2026-05-29 14:43:06.826] [INFO ] [main.cpp:18] No thread id

    // Custom combination — pick exactly what you need
    Logger::instance().set_format(LogFormat::Level | LogFormat::ThreadId | LogFormat::Message);
    LOG_INFO("Level + thread + message");
    // [INFO ] [tid:12345] Level + thread + message
}
```

### Recommended Patterns

```cpp
// For concurrency debugging: use kFull to see thread interleaving
Logger::instance().set_format(LogFormat::kFull);

// For algorithm/logic testing: use kCompact to focus on output
Logger::instance().set_format(LogFormat::kCompact);

// For production/file logging: use kDefault
Logger::instance().set_format(LogFormat::kDefault);
Logger::instance().set_output_file("app.log");
```

## File Output

Log to both terminal and file simultaneously:

```cpp
Logger::instance().set_output_file("debug.log");  // File has no ANSI colors
```

## Color Control

```cpp
Logger::instance().set_color_enabled(false);  // Disable colors (e.g., for piping to file)
```

Terminal color mapping:
- `DEBUG` → Cyan
- `INFO` → Green
- `WARNING` → Yellow
- `ERROR` → Red
- `FATAL` → Bold Magenta

## Formatted Logging Macros

For outputting variables, use the `_FMT` variants with stream syntax:

```cpp
std::string name = "GPT";
int params = 175;
LOG_INFO_FMT("Model: " << name << ", params: " << params << "B");
// [INFO ] Model: GPT, params: 175B
```

Available: `LOG_DEBUG_FMT`, `LOG_INFO_FMT`, `LOG_WARNING_FMT`, `LOG_ERROR_FMT`, `LOG_FATAL_FMT`

## Architecture

```
common/
├── include/common/
│   └── logger.h          # Public API: Logger class, LogLevel, LogFormat, macros
└── src/
    └── logger.cpp        # Implementation: formatting, file I/O, color codes
```

All executables in this project link against `common` and get the logger automatically:

```cmake
target_link_libraries(my_target PRIVATE common)
```
