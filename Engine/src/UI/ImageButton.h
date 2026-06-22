#pragma once

#include <memory>
#include <string>

#include <Paingine/Graphics/RenderWindow.hpp>
#include <Paingine/Graphics/Texture.hpp>
#include <Paingine/System/Vector2.hpp>

#include "BaseButton.h"

class ImageButton : public BaseButton {
public:
    ImageButton();
    ~ImageButton();

    using BaseButton::IsClicked;
    using BaseButton::IsMouseOver;

    void SetPosition(float x, float y);
    void SetPosition(Paingine2D::Vector2f position);

    void SetVisible(bool visible);

    virtual void SetTexture(const std::string& textureFile);
    void SetTexture(const Paingine2D::Texture& texture);
    void SetTexture(std::shared_ptr<Paingine2D::Texture> texture);

    bool IsMouseOver(Paingine2D::RenderWindow& window) const override;
    virtual bool IsClicked(Paingine2D::RenderWindow& window) override;

    void SetLayer(int layer);

protected:
    int m_Layer = 0;

private:
    void ApplyTexture(std::shared_ptr<Paingine2D::Texture> texture, const std::string& errorMessage);

    using Paingine2D::Sprite::setPosition;
};
