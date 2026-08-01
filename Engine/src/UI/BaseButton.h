#pragma once

#include <memory>
#include <string>

#include <Paingine/Graphics/RenderWindow.hpp>
#include <Paingine/Graphics/Sprite.hpp>
#include <Paingine/Graphics/Texture.hpp>

class BaseButton : public Paingine2D::Sprite {
public:
    virtual ~BaseButton() = default;

    virtual bool IsMouseOver() const;

    virtual bool IsMouseOver(Paingine2D::RenderWindow &window) const {
        (void) window;
        return false;
    }

    virtual bool IsClicked();

    virtual bool IsClicked(Paingine2D::RenderWindow &window) {
        (void) window;
        return false;
    }

    // Texture sources shared by every button type. Loading by name goes through
    // Resources; the const-reference overload copies, since the caller keeps ownership.
    virtual void SetTexture(const std::string &textureFile);
    void SetTexture(const Paingine2D::Texture &texture);
    void SetTexture(std::shared_ptr<Paingine2D::Texture> texture);

    // Layer the button draws on. Subclasses decide when it is registered.
    int GetLayer() const { return m_Layer; }

protected:
    bool HandleLeftClick(bool isMouseOver);

    bool SetOwnedTexture(std::shared_ptr<Paingine2D::Texture> texture);

    // Called after a texture is successfully applied, so subclasses can re-derive
    // anything that depends on the new bounds (origin, position, ...).
    virtual void OnTextureApplied() {
    }

    std::shared_ptr<Paingine2D::Texture> m_ButtonTexture;
    int m_Layer = 0;
    bool m_IsPressed = false;

private:
    void ApplyTexture(std::shared_ptr<Paingine2D::Texture> texture, const std::string &errorMessage);
};
