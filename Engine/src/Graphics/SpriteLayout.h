#pragma once

#include <Paingine/Graphics/Sprite.hpp>
#include <Paingine/System/Vector2.hpp>

namespace SpriteLayout {
    enum class FitMode {
        Contain,
        Cover
    };

    void SetOriginToCenter(Paingine2D::Sprite &sprite);
    void CenterAt(Paingine2D::Sprite &sprite, Paingine2D::Vector2f center);
    void FitToSize(Paingine2D::Sprite &sprite, Paingine2D::Vector2f size, FitMode mode);
    void FitToSizeCentered(Paingine2D::Sprite &sprite, Paingine2D::Vector2f size, FitMode mode);
}
