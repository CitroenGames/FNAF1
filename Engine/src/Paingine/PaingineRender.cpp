#include "Paingine/Paingine2D.h"

#include <SDL3/SDL.h>

namespace {
    [[nodiscard]] SDL_FRect ToSDLFRect(const Paingine2D::FloatRect &rect) {
        return SDL_FRect{rect.left, rect.top, rect.width, rect.height};
    }

    [[nodiscard]] SDL_FRect ToSDLFRect(const Paingine2D::IntRect &rect) {
        return SDL_FRect{
            static_cast<float>(rect.left),
            static_cast<float>(rect.top),
            static_cast<float>(rect.width),
            static_cast<float>(rect.height)};
    }
}

namespace Paingine2D {
View::View()
    : m_Center(0.0f, 0.0f),
      m_Size(0.0f, 0.0f) {
}

View::View(const FloatRect &rect)
    : m_Center(rect.left + rect.width * 0.5f, rect.top + rect.height * 0.5f),
      m_Size(rect.width, rect.height) {
}

void View::setCenter(const Vector2f &center) {
    m_Center = center;
}

void View::setCenter(float x, float y) {
    m_Center = Vector2f(x, y);
}

Vector2f View::getCenter() const {
    return m_Center;
}

void View::setSize(const Vector2f &size) {
    m_Size = size;
}

void View::setSize(float width, float height) {
    m_Size = Vector2f(width, height);
}

Vector2f View::getSize() const {
    return m_Size;
}

void View::setViewport(const FloatRect &viewport) {
    m_Viewport = viewport;
}

FloatRect View::getViewport() const {
    return m_Viewport;
}

void View::setRotation(float angle) {
    m_Rotation = angle;
}

float View::getRotation() const {
    return m_Rotation;
}

RenderTarget::RenderTarget(const Vector2u &size)
    : m_View(FloatRect(0.0f, 0.0f, static_cast<float>(size.x), static_cast<float>(size.y))),
      m_DefaultView(m_View),
      m_Size(size) {
}

void RenderTarget::draw(const Drawable &drawable) {
    drawable.draw(*this);
}

void RenderTarget::setView(const View &view) {
    m_View = view;
}

const View &RenderTarget::getView() const {
    return m_View;
}

const View &RenderTarget::getDefaultView() const {
    return m_DefaultView;
}

Vector2u RenderTarget::getSize() const {
    return m_Size;
}

Vector2f RenderTarget::mapPixelToCoords(const Vector2i &point) const {
    return mapPixelToCoords(point, m_View);
}

Vector2f RenderTarget::mapPixelToCoords(const Vector2i &point, const View &view) const {
    const FloatRect viewport = viewportPixels(view);
    if (viewport.width <= 0.0f || viewport.height <= 0.0f) {
        return Vector2f();
    }

    const Vector2f size = view.getSize();
    const Vector2f topLeft = view.getCenter() - size / 2.0f;
    const float normalizedX = (static_cast<float>(point.x) - viewport.left) / viewport.width;
    const float normalizedY = (static_cast<float>(point.y) - viewport.top) / viewport.height;
    return Vector2f(topLeft.x + normalizedX * size.x, topLeft.y + normalizedY * size.y);
}

Vector2i RenderTarget::mapCoordsToPixel(const Vector2f &point, const View &view) const {
    const FloatRect viewport = viewportPixels(view);
    const Vector2f size = view.getSize();
    if (size.x == 0.0f || size.y == 0.0f) {
        return Vector2i();
    }

    const Vector2f topLeft = view.getCenter() - size / 2.0f;
    const float normalizedX = (point.x - topLeft.x) / size.x;
    const float normalizedY = (point.y - topLeft.y) / size.y;
    return Vector2i(
        static_cast<int>(viewport.left + normalizedX * viewport.width),
        static_cast<int>(viewport.top + normalizedY * viewport.height));
}

void RenderTarget::setTargetSize(const Vector2u &size) {
    m_Size = size;
    m_DefaultView = View(FloatRect(0.0f, 0.0f, static_cast<float>(size.x), static_cast<float>(size.y)));
}

SDL_Renderer *RenderTarget::renderer() const {
    return m_Renderer;
}

void RenderTarget::setRenderer(SDL_Renderer *renderer) {
    m_Renderer = renderer;
}

FloatRect RenderTarget::worldToTargetRect(const FloatRect &worldRect) const {
    const FloatRect viewport = viewportPixels(m_View);
    const Vector2f viewSize = m_View.getSize();
    const Vector2f viewTopLeft = m_View.getCenter() - viewSize / 2.0f;

    if (viewSize.x == 0.0f || viewSize.y == 0.0f) {
        return FloatRect();
    }

    return FloatRect(
        viewport.left + ((worldRect.left - viewTopLeft.x) / viewSize.x) * viewport.width,
        viewport.top + ((worldRect.top - viewTopLeft.y) / viewSize.y) * viewport.height,
        (worldRect.width / viewSize.x) * viewport.width,
        (worldRect.height / viewSize.y) * viewport.height);
}

FloatRect RenderTarget::viewportPixels(const View &view) const {
    const FloatRect viewport = view.getViewport();
    return FloatRect(
        viewport.left * static_cast<float>(m_Size.x),
        viewport.top * static_cast<float>(m_Size.y),
        viewport.width * static_cast<float>(m_Size.x),
        viewport.height * static_cast<float>(m_Size.y));
}

Sprite::Sprite(const Texture &texture) {
    setTexture(texture, true);
}

void Sprite::setTexture(const Texture &texture, bool resetRect) {
    m_Texture = &texture;
    if (resetRect || m_TextureRect.width == 0 || m_TextureRect.height == 0) {
        const Vector2u size = texture.getSize();
        m_TextureRect = IntRect(0, 0, static_cast<int>(size.x), static_cast<int>(size.y));
    }
}

const Texture *Sprite::getTexture() const {
    return m_Texture;
}

void Sprite::setTextureRect(const IntRect &rect) {
    m_TextureRect = rect;
}

IntRect Sprite::getTextureRect() const {
    return m_TextureRect;
}

void Sprite::setPosition(float x, float y) {
    m_Position = Vector2f(x, y);
}

void Sprite::setPosition(const Vector2f &position) {
    m_Position = position;
}

Vector2f Sprite::getPosition() const {
    return m_Position;
}

void Sprite::setOrigin(float x, float y) {
    m_Origin = Vector2f(x, y);
}

void Sprite::setOrigin(const Vector2f &origin) {
    m_Origin = origin;
}

Vector2f Sprite::getOrigin() const {
    return m_Origin;
}

void Sprite::setScale(float x, float y) {
    m_Scale = Vector2f(x, y);
}

void Sprite::setScale(const Vector2f &scale) {
    m_Scale = scale;
}

Vector2f Sprite::getScale() const {
    return m_Scale;
}

void Sprite::setRotation(float angle) {
    m_Rotation = angle;
}

float Sprite::getRotation() const {
    return m_Rotation;
}

void Sprite::setColor(const Color &color) {
    m_Color = color;
}

Color Sprite::getColor() const {
    return m_Color;
}

FloatRect Sprite::getLocalBounds() const {
    return FloatRect(
        0.0f,
        0.0f,
        static_cast<float>(m_TextureRect.width),
        static_cast<float>(m_TextureRect.height));
}

FloatRect Sprite::getGlobalBounds() const {
    return FloatRect(
        m_Position.x - m_Origin.x * m_Scale.x,
        m_Position.y - m_Origin.y * m_Scale.y,
        static_cast<float>(m_TextureRect.width) * m_Scale.x,
        static_cast<float>(m_TextureRect.height) * m_Scale.y);
}

void Sprite::draw(RenderTarget &target) const {
    if (m_Texture == nullptr || target.renderer() == nullptr) {
        return;
    }

    SDL_Texture *texture = m_Texture->native(target.renderer());
    if (texture == nullptr) {
        return;
    }

    SDL_SetTextureColorMod(texture, m_Color.r, m_Color.g, m_Color.b);
    SDL_SetTextureAlphaMod(texture, m_Color.a);

    const SDL_FRect source = ToSDLFRect(m_TextureRect);
    const SDL_FRect destination = ToSDLFRect(target.worldToTargetRect(getGlobalBounds()));
    const SDL_FPoint center{m_Origin.x * m_Scale.x, m_Origin.y * m_Scale.y};
    SDL_RenderTextureRotated(
        target.renderer(),
        texture,
        &source,
        &destination,
        static_cast<double>(m_Rotation),
        &center,
        SDL_FLIP_NONE);
}

void Shape::setFillColor(const Color &color) {
    m_FillColor = color;
}

Color Shape::getFillColor() const {
    return m_FillColor;
}

RectangleShape::RectangleShape(const Vector2f &size)
    : m_Size(size) {
}

void RectangleShape::setSize(const Vector2f &size) {
    m_Size = size;
}

Vector2f RectangleShape::getSize() const {
    return m_Size;
}

void RectangleShape::setPosition(float x, float y) {
    m_Position = Vector2f(x, y);
}

void RectangleShape::setPosition(const Vector2f &position) {
    m_Position = position;
}

Vector2f RectangleShape::getPosition() const {
    return m_Position;
}

FloatRect RectangleShape::getLocalBounds() const {
    return FloatRect(0.0f, 0.0f, m_Size.x, m_Size.y);
}

FloatRect RectangleShape::getGlobalBounds() const {
    return FloatRect(m_Position.x, m_Position.y, m_Size.x, m_Size.y);
}

void RectangleShape::draw(RenderTarget &target) const {
    if (target.renderer() == nullptr) {
        return;
    }

    SDL_SetRenderDrawBlendMode(target.renderer(), SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(target.renderer(), m_FillColor.r, m_FillColor.g, m_FillColor.b, m_FillColor.a);
    const SDL_FRect rect = ToSDLFRect(target.worldToTargetRect(getGlobalBounds()));
    SDL_RenderFillRect(target.renderer(), &rect);
}

RenderTexture::RenderTexture()
    : RenderTarget(Vector2u(0, 0)) {
}
} // namespace Paingine2D
