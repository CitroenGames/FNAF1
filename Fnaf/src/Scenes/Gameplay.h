#pragma once

#include "Scene/Scene.h"
#include "Audio/AudioClip.h"
#include "Components/Camera.h"
#include "CameraSystem.h"
#include "Office.h"
#include "fnaf.hpp"
#include <array>
#include <vector>

class Gameplay : public Scene {
public:
    void Init() override;

    void FixedUpdate() override;

    void Update(double deltaTime) override;

    void Render() override;

    void Destroy() override;

private:
    std::shared_ptr<AudioClip> bgaudio1;
    std::shared_ptr<AudioClip> bgaudio2;

    // Office ambience currently follows the gameplay scene lifetime.
    std::shared_ptr<AudioClip> m_FanBuzzing;

    float scrollOffset = 175.0f; // Initial scroll offset

    std::unique_ptr<Camera2D> m_Camera;

    std::shared_ptr<CameraSystem> m_CameraSystem;
    std::shared_ptr<HUDButton> m_CameraButton;

    std::shared_ptr<FNAFGame> gameplay;

    std::shared_ptr<Office> m_OfficeComponent;

    // Power HUD
    std::shared_ptr<Paingine2D::Texture> m_PowerLeftTexture;
    std::shared_ptr<Paingine2D::Texture> m_UsageLabelTexture;
    std::array<std::shared_ptr<Paingine2D::Texture>, 5> m_UsageBarTextures;
    Paingine2D::Sprite m_PowerLeftSprite;
    Paingine2D::Sprite m_UsageLabelSprite;
    Paingine2D::Sprite m_UsageBarsSprite;
    std::shared_ptr<Paingine2D::Font> m_PowerFont;
    Paingine2D::Text m_PowerPercentText;

    enum class DeathSequenceState {
        None,
        Jumpscare,
        GameOver
    };

    DeathSequenceState m_DeathSequenceState = DeathSequenceState::None;
    JumpscareType m_ActiveJumpscare = JumpscareType::None;
    std::vector<std::shared_ptr<Paingine2D::Texture>> m_JumpscareFrames;
    Paingine2D::Sprite m_JumpscareSprite;
    std::shared_ptr<AudioClip> m_JumpscareSound;
    float m_JumpscareFrameTimer = 0.0f;
    float m_JumpscareTotalTimer = 0.0f;
    float m_JumpscareFrameDuration = 1.0f / 30.0f;
    float m_JumpscareMinimumDuration = 1.0f;
    std::size_t m_JumpscareFrameIndex = 0;
    bool m_DiscardNextJumpscareDelta = false;

    std::shared_ptr<Paingine2D::Texture> m_GameOverBackgroundTexture;
    std::shared_ptr<Paingine2D::Texture> m_GameOverTextTexture;
    Paingine2D::Sprite m_GameOverBackgroundSprite;
    Paingine2D::Sprite m_GameOverTextSprite;
    float m_GameOverTimer = 0.0f;

    void StartPendingJumpscare();
    void StartJumpscare(JumpscareType type);
    void UpdateDeathSequence(float deltaTime);
    void SwitchToGameOver();
    void DrawDeathSequence(Paingine2D::RenderWindow &window);
    bool IsDeathSequenceActive() const { return m_DeathSequenceState != DeathSequenceState::None; }
};
