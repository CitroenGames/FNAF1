#pragma once

#include <memory>
#include <vector>

#include <Paingine/Graphics/Sprite.hpp>
#include <Paingine/Graphics/Texture.hpp>

class FlipBook {
public:
    FlipBook();
    ~FlipBook();
    FlipBook(int layer, float frameDuration = 0.1f, bool loop = true);

    // Add frames to the flipbook
    void AddFrame(std::shared_ptr<Paingine2D::Texture> texture);
    void AddFrame(std::shared_ptr<Paingine2D::Sprite> sprite);
    void AddFrames(const std::vector<std::shared_ptr<Paingine2D::Sprite>>& sprites);
    void AddFrames(const std::vector<Paingine2D::Texture>& textures);

    // Animation control
    void Update(float deltaTime);
    void Play(bool forward = true);
    void Pause();
    void Stop();
    void Cleanup();

    // Layer management
    void RegisterToLayerManager();
    void UnregisterFromLayerManager();

    // Setters
    void SetPosition(float x, float y);
    void SetFrameDuration(float duration);
    void SetLoop(bool shouldLoop);

    // Getters
    bool IsPlaying() const;
    Paingine2D::Sprite* GetCurrentFrame();
    const Paingine2D::Sprite* GetCurrentFrame() const;

private:
    std::vector<std::shared_ptr<Paingine2D::Sprite>> m_Frames;
    float m_FrameDuration;
    float m_ElapsedTime;
    std::size_t m_CurrentFrame;
    bool m_IsPlayingFlag;
    bool m_Loop;
    int m_Layer;
    bool m_IsForward;

    void AddSpriteFrame(std::shared_ptr<Paingine2D::Sprite> sprite);
    bool HasCurrentFrame() const;
    void UpdateLayerManagerRegistration();
};
