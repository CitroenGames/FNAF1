#pragma once

#include <Paingine/Graphics/Drawable.hpp>
#include <Paingine/System/Time.hpp>

class FadeEffect {
public:
    enum FadeState { In, Out, None };

    FadeEffect();
    ~FadeEffect();

    void SetDrawable(Paingine2D::Drawable* drawable);
    void SetDuration(const Paingine2D::Time& duration);

    void StartFadeIn();
    void StartFadeOut();

    bool Update(Paingine2D::Time deltaTime);
    bool IsFading() const;

private:
    Paingine2D::Drawable* m_Drawable = nullptr;
    Paingine2D::Time m_Duration;
    FadeState m_State;
    Paingine2D::Time m_ElapsedTime;
    Paingine2D::Uint8 m_Alpha;

    void ApplyAlpha();
};
