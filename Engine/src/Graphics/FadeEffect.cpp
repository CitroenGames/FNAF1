#include "Graphics/FadeEffect.h"

#include <Paingine/Graphics/RectangleShape.hpp>
#include <Paingine/Graphics/Shape.hpp>
#include <Paingine/Graphics/Sprite.hpp>
#include <Paingine/Graphics/Text.hpp>

FadeEffect::FadeEffect()
    : m_Duration(Paingine2D::seconds(1.0f))
    , m_State(None)
    , m_ElapsedTime(Paingine2D::Time::Zero)
    , m_Alpha(0) {
}

FadeEffect::~FadeEffect() {
    m_Drawable = nullptr;
}

void FadeEffect::SetDrawable(Paingine2D::Drawable* drawable) {
    m_Drawable = drawable;
}

void FadeEffect::SetDuration(const Paingine2D::Time& duration) {
    m_Duration = duration;
}

void FadeEffect::StartFadeIn() {
    m_State = In;
    m_ElapsedTime = Paingine2D::Time::Zero;
    m_Alpha = 0;
}

void FadeEffect::StartFadeOut() {
    m_State = Out;
    m_ElapsedTime = Paingine2D::Time::Zero;
    m_Alpha = 255;
}

bool FadeEffect::Update(Paingine2D::Time deltaTime) {
    if (m_State == None || !m_Drawable) {
        return false;
    }

    m_ElapsedTime += deltaTime;

    // A zero-length fade completes immediately rather than dividing by zero.
    const float durationSeconds = m_Duration.asSeconds();
    const float progress = durationSeconds > 0.0f
        ? m_ElapsedTime.asSeconds() / durationSeconds
        : 1.0f;

    if (progress >= 1.0f) {
        if (m_State == In) {
            m_Alpha = 255;
        } else if (m_State == Out) {
            m_Alpha = 0;
        }
        ApplyAlpha();
        m_State = None;
        return true;
    }

    if (m_State == In) {
        m_Alpha = static_cast<Paingine2D::Uint8>(255 * progress);
    } else if (m_State == Out) {
        m_Alpha = static_cast<Paingine2D::Uint8>(255 * (1.0f - progress));
    }

    ApplyAlpha();
    return false;
}

bool FadeEffect::IsFading() const {
    return m_State != None;
}

void FadeEffect::ApplyAlpha() {
    if (!m_Drawable) {
        return;
    }

    // RectangleShape derives from Shape, so the Shape branch already covers it.
    if (auto *shape = dynamic_cast<Paingine2D::Shape *>(m_Drawable)) {
        Paingine2D::Color color = shape->getFillColor();
        color.a = m_Alpha;
        shape->setFillColor(color);
    } else if (auto *sprite = dynamic_cast<Paingine2D::Sprite *>(m_Drawable)) {
        Paingine2D::Color color = sprite->getColor();
        color.a = m_Alpha;
        sprite->setColor(color);
    } else if (auto *text = dynamic_cast<Paingine2D::Text *>(m_Drawable)) {
        Paingine2D::Color color = text->getFillColor();
        color.a = m_Alpha;
        text->setFillColor(color);
    }
}
