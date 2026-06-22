#pragma once

#include <memory>

#include <Paingine/Graphics/RenderWindow.hpp>
#include <Paingine/Graphics/Sprite.hpp>
#include <Paingine/Graphics/Texture.hpp>

class BaseButton : public Paingine2D::Sprite {
public:
    virtual ~BaseButton() = default;

    virtual bool IsMouseOver() const;

    virtual bool IsMouseOver(Paingine2D::RenderWindow& window) const {
        (void) window;
        return false;
    }

    virtual bool IsClicked();

    virtual bool IsClicked(Paingine2D::RenderWindow& window) {
        (void) window;
        return false;
    }

protected:
    bool HandleLeftClick(bool isMouseOver);

    bool SetOwnedTexture(std::shared_ptr<Paingine2D::Texture> texture);

    std::shared_ptr<Paingine2D::Texture> m_ButtonTexture;
    bool m_IsPressed = false;
};
