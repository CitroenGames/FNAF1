#include "UI/HUDButton.h"

#include <iostream>
#include <utility>

#include <Paingine/Graphics/Rect.hpp>
#include <Paingine/Window/Mouse.hpp>

#include "Assets/Resources.h"
#include "Core/Window.h"
#include "Graphics/LayerManager.h"
#include "Graphics/ScopedView.h"

HUDButton::HUDButton() = default;

HUDButton::~HUDButton() {
    LayerManager::RemoveDrawable(this);
}

void HUDButton::SetPosition(float x, float y) {
    m_ScreenPosition.x = x;
    m_ScreenPosition.y = y;
    UpdatePosition();
}

void HUDButton::SetPosition(Paingine2D::Vector2f position) {
    m_ScreenPosition = position;
    UpdatePosition();
}

Paingine2D::Vector2f HUDButton::GetScreenPosition() const {
    return m_ScreenPosition;
}

void HUDButton::SetTexture(const std::string& textureFile) {
    ApplyTexture(Resources::GetTexture(textureFile), "Error loading texture: " + textureFile);
}

void HUDButton::SetTexture(const Paingine2D::Texture& texture) {
    ApplyTexture(std::make_shared<Paingine2D::Texture>(texture), "Error: Provided texture is null.");
}

void HUDButton::SetTexture(std::shared_ptr<Paingine2D::Texture> texture) {
    ApplyTexture(std::move(texture), "Error: Provided texture is null.");
}

bool HUDButton::IsMouseOver(Paingine2D::RenderWindow& window) const {
    const Paingine2D::View currentView = window.getView();
    const Paingine2D::Vector2i mousePos = Paingine2D::Mouse::getPosition(window);
    const Paingine2D::FloatRect viewport = currentView.getViewport();
    const Paingine2D::Vector2u windowSize = window.getSize();
    ScopedView screenView(window, window.getDefaultView());

    Paingine2D::Vector2f viewPos;
    if (viewport.width < 1.0f) {
        if (mousePos.x < viewport.left * windowSize.x ||
            mousePos.x > (viewport.left + viewport.width) * windowSize.x) {
            return false;
        }

        float adjustedX = (mousePos.x - (viewport.left * windowSize.x)) /
                          (viewport.width * windowSize.x) * windowSize.x;
        viewPos = window.mapPixelToCoords(Paingine2D::Vector2i(static_cast<int>(adjustedX), mousePos.y));
    } else if (viewport.height < 1.0f) {
        if (mousePos.y < viewport.top * windowSize.y ||
            mousePos.y > (viewport.top + viewport.height) * windowSize.y) {
            return false;
        }

        float adjustedY = (mousePos.y - (viewport.top * windowSize.y)) /
                          (viewport.height * windowSize.y) * windowSize.y;
        viewPos = window.mapPixelToCoords(Paingine2D::Vector2i(mousePos.x, static_cast<int>(adjustedY)));
    } else {
        viewPos = window.mapPixelToCoords(mousePos);
    }

    const Paingine2D::FloatRect buttonBounds = getGlobalBounds();
    return buttonBounds.contains(viewPos);
}

bool HUDButton::IsClicked(Paingine2D::RenderWindow& window) {
    return HandleLeftClick(IsMouseOver(window));
}

void HUDButton::UpdatePosition() {
    Paingine2D::Sprite::setPosition(AdjustForViewport(m_ScreenPosition));
}

void HUDButton::Draw(Paingine2D::RenderWindow& window) {
    ScopedView screenView(window, window.getDefaultView());

    Paingine2D::Vector2f originalPos = Paingine2D::Sprite::getPosition();
    Paingine2D::Sprite::setPosition(AdjustForViewport(m_ScreenPosition));

    window.draw(*this);

    Paingine2D::Sprite::setPosition(originalPos);
}

void HUDButton::Show() {
    if (!m_IsVisible) {
        m_IsVisible = true;
        LayerManager::AddDrawable(m_Layer, this);
    }
}

void HUDButton::Hide() {
    if (m_IsVisible) {
        m_IsVisible = false;
        LayerManager::RemoveDrawable(this);
    }
}

void HUDButton::CenterOrigin() {
    const Paingine2D::FloatRect bounds = getLocalBounds();
    setOrigin(bounds.width / 2.f, bounds.height / 2.f);
    UpdatePosition();
}

Paingine2D::Vector2f HUDButton::AdjustForViewport(const Paingine2D::Vector2f& position) const {
    auto window = Window::GetWindow();
    if (!window) {
        return position;
    }

    Paingine2D::View currentView = window->getView();
    Paingine2D::FloatRect viewport = currentView.getViewport();
    Paingine2D::Vector2u windowSize = window->getSize();

    Paingine2D::Vector2f adjustedPos = position;
    if (viewport.width < 1.0f) {
        adjustedPos.x = (viewport.left * windowSize.x) + (position.x * viewport.width);
    }

    if (viewport.height < 1.0f) {
        adjustedPos.y = (viewport.top * windowSize.y) + (position.y * viewport.height);
    }

    return adjustedPos;
}

void HUDButton::ApplyTexture(std::shared_ptr<Paingine2D::Texture> texture, const std::string& errorMessage) {
    if (SetOwnedTexture(std::move(texture))) {
        CenterOrigin();
    } else {
        std::cerr << errorMessage << std::endl;
    }
}
