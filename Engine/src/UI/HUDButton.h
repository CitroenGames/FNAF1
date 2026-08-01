#pragma once

#include <Paingine/Graphics/RenderWindow.hpp>
#include <Paingine/System/Vector2.hpp>

#include "BaseButton.h"

// A button anchored to screen space: its position is given in design-resolution
// pixels and remapped through the active viewport, so letterboxing does not move it.
class HUDButton : public BaseButton {
public:
    HUDButton();
    ~HUDButton() override;

    using BaseButton::IsClicked;
    using BaseButton::IsMouseOver;
    using BaseButton::SetTexture;

    void SetPosition(float x, float y);
    void SetPosition(Paingine2D::Vector2f position);

    Paingine2D::Vector2f GetScreenPosition() const;

    bool IsMouseOver(Paingine2D::RenderWindow &window) const override;
    bool IsClicked(Paingine2D::RenderWindow &window) override;

    void UpdatePosition();
    void Draw(Paingine2D::RenderWindow &window);
    void Show();
    void Hide();

protected:
    // Re-centres the origin whenever a new texture changes the bounds.
    void OnTextureApplied() override;

    void CenterOrigin();
    Paingine2D::Vector2f AdjustForViewport(const Paingine2D::Vector2f &position) const;

    Paingine2D::Vector2f m_ScreenPosition; // Position relative to screen
    bool m_IsVisible = false;

private:
    using Paingine2D::Sprite::setPosition;
};
