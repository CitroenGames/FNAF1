#pragma once

#include <memory>
#include <vector>

#include <Paingine/Graphics/Sprite.hpp>
#include <Paingine/Graphics/Texture.hpp>

class GlitchEffect {
private:
    std::vector<std::shared_ptr<Paingine2D::Sprite>> m_Frames;
    int m_Layer;
    int m_CurrentFrame;
    int m_PreviousFrame;  // Track previous frame for proper cleanup
    bool m_IsRunning;

    // Glitch effect parameters
    bool m_IsGlitching;
    float m_GlitchTimer;
    float m_GlitchChance;        // Chance to start a glitch
    float m_MultiGlitchChance;   // Chance for multiple random frames
    float m_GlitchDuration;      // How long each glitch frame shows
    std::vector<int> m_GlitchSequence;  // Sequence of frames to show during glitch

public:
    GlitchEffect();
    GlitchEffect(int layer);
    ~GlitchEffect();

    void Stop();
    void Kill();

    void AddFrame(std::shared_ptr<Paingine2D::Texture> texture);
    void AddFrame(std::shared_ptr<Paingine2D::Sprite> sprite);
    void AddFrames(const std::vector<std::shared_ptr<Paingine2D::Sprite>>& sprites);
    void AddFrames(const std::vector<std::shared_ptr<Paingine2D::Texture>>& textures);

    void SetPosition(float x, float y);
    void SetLayer(int layer);
    void Update();

private:
    void RegisterToLayerManager();
    void UpdateLayerManager();

public:
    void StartGlitch();
    void SetGlitchParameters(float chance, float multiChance, float duration);
};
