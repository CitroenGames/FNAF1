#include "UI/HUDButton.h"

#include <Paingine/Graphics/Rect.hpp>
#include <Paingine/Window/Mouse.hpp>

#include "Core/Window.h"
#include "Graphics/LayerManager.h"
#include "Graphics/ScopedView.h"

HUDButton::HUDButton() = default;

HUDButton::~HUDButton() {
    LayerManager::RemoveDrawable(this);
}

void HUDButton::SetPosition(float x, float y) {
    SetPosition(Paingine2D::Vector2f(x, y));
}

void HUDButton::SetPosition(Paingine2D::Vector2f position) {
    m_ScreenPosition = position;
    UpdatePosition();
}

Paingine2D::Vector2f HUDButton::GetScreenPosition() const {
    return m_ScreenPosition;
}

bool HUDButton::IsMouseOver(Paingine2D::RenderWindow &window) const {
    const Paingine2D::FloatRect viewport = window.getView().getViewport();
    const Paingine2D::Vector2i mousePos = Paingine2D::Mouse::getPosition(window);
    const Paingine2D::Vector2u windowSize = window.getSize();
    ScopedView screenView(window, window.getDefaultView());

    // Letterboxing shrinks the viewport along one axis; undo that so the cursor is
    // compared against bounds expressed in full-window screen space.
    Paingine2D::Vector2f viewPos;
    if (viewport.width < 1.0f) {
        const float left = viewport.left * windowSize.x;
        const float width = viewport.width * windowSize.x;
        if (mousePos.x < left || mousePos.x > left + width) {
            return false;
        }

        const float adjustedX = (mousePos.x - left) / width * windowSize.x;
        viewPos = window.mapPixelToCoords(Paingine2D::Vector2i(static_cast<int>(adjustedX), mousePos.y));
    } else if (viewport.height < 1.0f) {
        const float top = viewport.top * windowSize.y;
        const float height = viewport.height * windowSize.y;
        if (mousePos.y < top || mousePos.y > top + height) {
            return false;
        }

        const float adjustedY = (mousePos.y - top) / height * windowSize.y;
        viewPos = window.mapPixelToCoords(Paingine2D::Vector2i(mousePos.x, static_cast<int>(adjustedY)));
    } else {
        viewPos = window.mapPixelToCoords(mousePos);
    }

    return getGlobalBounds().contains(viewPos);
}

bool HUDButton::IsClicked(Paingine2D::RenderWindow &window) {
    return HandleLeftClick(IsMouseOver(window));
}

void HUDButton::UpdatePosition() {
    Paingine2D::Sprite::setPosition(AdjustForViewport(m_ScreenPosition));
}

void HUDButton::Draw(Paingine2D::RenderWindow &window) {
    ScopedView screenView(window, window.getDefaultView());

    const Paingine2D::Vector2f originalPos = Paingine2D::Sprite::getPosition();
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

void HUDButton::OnTextureApplied() {
    CenterOrigin();
}

void HUDButton::CenterOrigin() {
    const Paingine2D::FloatRect bounds = getLocalBounds();
    setOrigin(bounds.width / 2.f, bounds.height / 2.f);
    UpdatePosition();
}

Paingine2D::Vector2f HUDButton::AdjustForViewport(const Paingine2D::Vector2f &position) const {
    const auto window = Window::GetWindow();
    if (!window) {
        return position;
    }

    const Paingine2D::FloatRect viewport = window->getView().getViewport();
    const Paingine2D::Vector2u windowSize = window->getSize();

    Paingine2D::Vector2f adjustedPos = position;
    if (viewport.width < 1.0f) {
        adjustedPos.x = (viewport.left * windowSize.x) + (position.x * viewport.width);
    }

    if (viewport.height < 1.0f) {
        adjustedPos.y = (viewport.top * windowSize.y) + (position.y * viewport.height);
    }

    return adjustedPos;
}
