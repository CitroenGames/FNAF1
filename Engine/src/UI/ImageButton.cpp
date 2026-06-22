#include "UI/ImageButton.h"

#include <iostream>
#include <utility>

#include <Paingine/Graphics/Rect.hpp>

#include "Assets/Resources.h"
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

void ImageButton::SetTexture(const std::string& textureFile) {
    ApplyTexture(Resources::GetTexture(textureFile), "Error loading texture: " + textureFile);
}

void ImageButton::SetTexture(const Paingine2D::Texture& texture) {
    ApplyTexture(std::make_shared<Paingine2D::Texture>(texture), "Error: Provided texture is null.");
}

void ImageButton::SetTexture(std::shared_ptr<Paingine2D::Texture> texture) {
    ApplyTexture(std::move(texture), "Error: Provided texture is null.");
}

bool ImageButton::IsMouseOver(Paingine2D::RenderWindow& window) const {
    Paingine2D::Vector2i mousePos = Paingine2D::Mouse::getPosition(window);
    Paingine2D::Vector2f viewPos = window.mapPixelToCoords(mousePos);
    Paingine2D::FloatRect buttonBounds = getGlobalBounds();
    return buttonBounds.contains(viewPos);
}

bool ImageButton::IsClicked(Paingine2D::RenderWindow& window) {
    return HandleLeftClick(IsMouseOver(window));
}

void ImageButton::SetLayer(int layer) {
    if (m_Layer != layer) {
        LayerManager::ChangeLayer(this, layer);
        m_Layer = layer;
    }
}

void ImageButton::ApplyTexture(std::shared_ptr<Paingine2D::Texture> texture, const std::string& errorMessage) {
    if (!SetOwnedTexture(std::move(texture))) {
        std::cerr << errorMessage << std::endl;
    }
}
