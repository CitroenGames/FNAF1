#include "UI/BaseButton.h"

#include <utility>

#include <Paingine/Window/Mouse.hpp>

#include "Core/Window.h"

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
