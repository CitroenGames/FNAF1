#include "UI/ImageButton.h"

#include <Paingine/Graphics/Rect.hpp>
#include <Paingine/Window/Mouse.hpp>

#include "Graphics/LayerManager.h"

ImageButton::ImageButton() {
    LayerManager::AddDrawable(m_Layer, this);
}

ImageButton::~ImageButton() {
    LayerManager::RemoveDrawable(this);
}

void ImageButton::SetPosition(float x, float y) {
    Paingine2D::Sprite::setPosition(x, y);
}

void ImageButton::SetPosition(Paingine2D::Vector2f position) {
    Paingine2D::Sprite::setPosition(position);
}

void ImageButton::SetVisible(bool visible) {
    if (visible) {
        LayerManager::AddDrawable(m_Layer, this);
    } else {
        LayerManager::RemoveDrawable(this);
    }
}

bool ImageButton::IsMouseOver(Paingine2D::RenderWindow &window) const {
    const Paingine2D::Vector2i mousePos = Paingine2D::Mouse::getPosition(window);
    const Paingine2D::Vector2f viewPos = window.mapPixelToCoords(mousePos);
    return getGlobalBounds().contains(viewPos);
}

bool ImageButton::IsClicked(Paingine2D::RenderWindow &window) {
    return HandleLeftClick(IsMouseOver(window));
}

void ImageButton::SetLayer(int layer) {
    if (m_Layer != layer) {
        LayerManager::ChangeLayer(this, layer);
        m_Layer = layer;
    }
}
