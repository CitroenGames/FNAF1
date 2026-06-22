// Add this to your imconfig.h

#include <Paingine/Graphics/Color.hpp>
#include <Paingine/System/Vector2.hpp>

#include <cstdint>

#include "imgui-Paingine_export.h"

#define IM_VEC2_CLASS_EXTRA                                          \
    template<typename T>                                             \
    ImVec2(const Paingine2D::Vector2<T>& v) {                                \
        x = static_cast<float>(v.x);                                 \
        y = static_cast<float>(v.y);                                 \
    }                                                                \
                                                                     \
    template<typename T>                                             \
    operator Paingine2D::Vector2<T>() const {                                \
        return Paingine2D::Vector2<T>(static_cast<T>(x), static_cast<T>(y)); \
    }

#define IM_VEC4_CLASS_EXTRA                                                                       \
    ImVec4(const Paingine2D::Color& c) : x(c.r / 255.f), y(c.g / 255.f), z(c.b / 255.f), w(c.a / 255.f) { \
    }                                                                                             \
    operator Paingine2D::Color() const {                                                                  \
        return Paingine2D::Color(static_cast<std::uint8_t>(x * 255.f),                                    \
                         static_cast<std::uint8_t>(y * 255.f),                                    \
                         static_cast<std::uint8_t>(z * 255.f),                                    \
                         static_cast<std::uint8_t>(w * 255.f));                                   \
    }
