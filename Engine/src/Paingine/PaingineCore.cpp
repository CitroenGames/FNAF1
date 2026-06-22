#include "Paingine/Paingine2D.h"

#include <SDL3/SDL.h>

namespace {
    [[nodiscard]] double NowSeconds() {
        return static_cast<double>(SDL_GetPerformanceCounter()) /
               static_cast<double>(SDL_GetPerformanceFrequency());
    }
}

namespace Paingine2D {
const Color Color::Black{0, 0, 0, 255};
const Color Color::White{255, 255, 255, 255};
const Color Color::Transparent{0, 0, 0, 0};
const Time Time::Zero{0.0f};

Time seconds(float value) {
    return Time(value);
}

Clock::Clock()
    : m_LastSeconds(NowSeconds()) {
}

Time Clock::restart() {
    const double current = NowSeconds();
    const double elapsed = current - m_LastSeconds;
    m_LastSeconds = current;
    return Time(static_cast<float>(elapsed));
}
} // namespace Paingine2D
