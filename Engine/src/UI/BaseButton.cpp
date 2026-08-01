#include "UI/BaseButton.h"

#include <utility>

#include <Paingine/Window/Mouse.hpp>

#include "Assets/Resources.h"
#include "Core/Window.h"
#include "Utils/Log.h"

bool BaseButton::IsMouseOver() const {
    const auto window = Window::GetWindow();
    return window && IsMouseOver(*window);
}

bool BaseButton::IsClicked() {
    const auto window = Window::GetWindow();
    return window && IsClicked(*window);
}

bool BaseButton::HandleLeftClick(bool isMouseOver) {
    const bool isCurrentlyPressed = isMouseOver && Paingine2D::Mouse::isButtonPressed(Paingine2D::Mouse::Left);
    if (isCurrentlyPressed && !m_IsPressed) {
        m_IsPressed = true;
        return true;
    }

    if (!isCurrentlyPressed) {
        m_IsPressed = false;
    }

    return false;
}

bool BaseButton::SetOwnedTexture(std::shared_ptr<Paingine2D::Texture> texture) {
    m_ButtonTexture = std::move(texture);
    if (!m_ButtonTexture) {
        return false;
    }

    Paingine2D::Sprite::setTexture(*m_ButtonTexture);
    return true;
}

void BaseButton::SetTexture(const std::string &textureFile) {
    ApplyTexture(Resources::GetTexture(textureFile), "failed to load texture " + textureFile);
}

void BaseButton::SetTexture(const Paingine2D::Texture &texture) {
    ApplyTexture(std::make_shared<Paingine2D::Texture>(texture), "provided texture is null");
}

void BaseButton::SetTexture(std::shared_ptr<Paingine2D::Texture> texture) {
    ApplyTexture(std::move(texture), "provided texture is null");
}

void BaseButton::ApplyTexture(std::shared_ptr<Paingine2D::Texture> texture, const std::string &errorMessage) {
    if (!SetOwnedTexture(std::move(texture))) {
        Log::Error("Button", errorMessage);
        return;
    }

    OnTextureApplied();
}
