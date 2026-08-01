#pragma once

#include <string>
#include <string_view>

// Minimal levelled logger. Engine code should route diagnostics through here
// instead of writing to std::cerr directly, so that verbosity is controllable
// and every message carries a consistent scope tag.
namespace Log {
    enum class Level {
        Trace,
        Info,
        Warn,
        Error,
        Off
    };

    void SetLevel(Level level);

    Level GetLevel();

    bool IsEnabled(Level level);

    void Write(Level level, std::string_view scope, std::string_view message);

    inline void Trace(std::string_view scope, std::string_view message) {
        Write(Level::Trace, scope, message);
    }

    inline void Info(std::string_view scope, std::string_view message) {
        Write(Level::Info, scope, message);
    }

    inline void Warn(std::string_view scope, std::string_view message) {
        Write(Level::Warn, scope, message);
    }

    inline void Error(std::string_view scope, std::string_view message) {
        Write(Level::Error, scope, message);
    }
}
