#pragma once

#include <Paingine/Graphics/Rect.hpp>
#include <Paingine/System/Vector2.hpp>

namespace UI::Layout {
    inline Paingine2D::Vector2f CenteredPosition(Paingine2D::Vector2f areaSize, const Paingine2D::FloatRect &bounds) {
        return {
            (areaSize.x - bounds.width) * 0.5f - bounds.left,
            (areaSize.y - bounds.height) * 0.5f - bounds.top
        };
    }

    inline Paingine2D::Vector2f CenteredPosition(const Paingine2D::Vector2u &areaSize, const Paingine2D::FloatRect &bounds) {
        return CenteredPosition(
            Paingine2D::Vector2f(static_cast<float>(areaSize.x), static_cast<float>(areaSize.y)),
            bounds);
    }

    inline float CenteredX(float areaWidth, const Paingine2D::FloatRect &bounds) {
        return (areaWidth - bounds.width) * 0.5f - bounds.left;
    }

    inline float CenteredX(const Paingine2D::Vector2u &areaSize, const Paingine2D::FloatRect &bounds) {
        return CenteredX(static_cast<float>(areaSize.x), bounds);
    }
}
