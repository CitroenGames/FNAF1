#pragma once

#include <Paingine/Graphics/RenderWindow.hpp>
#include <Paingine/System/Vector2.hpp>

#include "BaseButton.h"

// A button positioned in world/view coordinates and drawn by the LayerManager.
class ImageButton : public BaseButton {
public:
    ImageButton();
    ~ImageButton() override;

    using BaseButton::IsClicked;
    using BaseButton::IsMouseOver;
    using BaseButton::SetTexture;

    void SetPosition(float x, float y);
    void SetPosition(Paingine2D::Vector2f position);

    void SetVisible(bool visible);

    bool IsMouseOver(Paingine2D::RenderWindow &window) const override;
    bool IsClicked(Paingine2D::RenderWindow &window) override;

    void SetLayer(int layer);

private:
    using Paingine2D::Sprite::setPosition;
};
