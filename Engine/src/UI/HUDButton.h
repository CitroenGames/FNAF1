#pragma once

#include <memory>
#include <string>

#include <Paingine/Graphics/RenderWindow.hpp>
#include <Paingine/Graphics/Texture.hpp>
#include <Paingine/System/Vector2.hpp>

#include "BaseButton.h"

class HUDButton : public BaseButton {
public:
    HUDButton();
    ~HUDButton();

    using BaseButton::IsClicked;
    using BaseButton::IsMouseOver;

    void SetPosition(float x, float y);
    void SetPosition(Paingine2D::Vector2f position);

    Paingine2D::Vector2f GetScreenPosition() const;

    virtual void SetTexture(const std::string& textureFile);
    void SetTexture(const Paingine2D::Texture& texture);
    void SetTexture(std::shared_ptr<Paingine2D::Texture> texture);

    bool IsMouseOver(Paingine2D::RenderWindow& window) const override;
    bool IsClicked(Paingine2D::RenderWindow& window) override;

    void UpdatePosition();
    void Draw(Paingine2D::RenderWindow& window);
    void Show();
    void Hide();

protected:
    int m_Layer = 0;
    Paingine2D::Vector2f m_ScreenPosition; // Position relative to screen
    bool m_IsVisible = false;

    void CenterOrigin();
    Paingine2D::Vector2f AdjustForViewport(const Paingine2D::Vector2f& position) const;

private:
    void ApplyTexture(std::shared_ptr<Paingine2D::Texture> texture, const std::string& errorMessage);

    using Paingine2D::Sprite::setPosition;
};
