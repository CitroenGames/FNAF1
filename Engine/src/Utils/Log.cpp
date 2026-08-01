#include "Utils/Log.h"

#include <iostream>
#include <mutex>

namespace {
    Log::Level g_MinimumLevel = Log::Level::Info;
    std::mutex g_Mutex;

    std::string_view LevelTag(Log::Level level) {
        switch (level) {
            case Log::Level::Trace:
                return "TRACE";
            case Log::Level::Info:
                return "INFO";
            case Log::Level::Warn:
                return "WARN";
            case Log::Level::Error:
                return "ERROR";
            case Log::Level::Off:
                break;
        }

        return "?";
    }
}

namespace Log {
    void SetLevel(Level level) {
        std::lock_guard<std::mutex> lock(g_Mutex);
        g_MinimumLevel = level;
    }

    Level GetLevel() {
        std::lock_guard<std::mutex> lock(g_Mutex);
        return g_MinimumLevel;
    }

    bool IsEnabled(Level level) {
        return level != Level::Off && level >= GetLevel();
    }

    void Write(Level level, std::string_view scope, std::string_view message) {
        if (!IsEnabled(level)) {
            return;
        }

        std::ostream &stream = level >= Level::Warn ? std::cerr : std::cout;

        std::lock_guard<std::mutex> lock(g_Mutex);
        stream << '[' << LevelTag(level) << "] " << scope << ": " << message << '\n';
        if (level >= Level::Warn) {
            stream.flush();
        }
    }
}
