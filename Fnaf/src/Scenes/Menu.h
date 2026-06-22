#pragma once

#include "Scene/Scene.h"
#include "Animation/GlitchEffect.h"
#include "Graphics/LayerManager.h"
#include "UI/ImageButton.h"

inline constexpr const char *STATIC_AUDIO_KEY = "Audio/static2.wav";
inline constexpr const char *MENU_MUSIC_KEY = "Audio/Menu/darknessmusic.wav";

class Menu : public Scene
{
public:
    Menu();
    void Init() override;
    void Update(double deltaTime) override;
    void FixedUpdate() override;
    void Render() override {}
    void Destroy() override {LayerManager::Clear();}
    void SwitchToGameplay();

    void HideAllMenuElements();
    void ShowMainMenuElements();

    // Methods to handle glitch effects
    void HideGlitchEffects();
    void ShowGlitchEffects();
private:
    // Textures
    std::shared_ptr<Paingine2D::Texture> m_Logo;
    std::shared_ptr<Paingine2D::Texture> NewsPaperTexture;
    std::shared_ptr<Paingine2D::Texture> m_WarningMessageTexture;
    std::shared_ptr<Paingine2D::Texture> m_LoadingScreenTexture;

    Paingine2D::Sprite m_LogoSprite;
    Paingine2D::Sprite NewsPaperSprite;
    Paingine2D::Sprite m_WarningMessageSprite;
    Paingine2D::Sprite m_LoadingScreenSprite;
    Paingine2D::Text m_TimeText;
    Paingine2D::Text m_NightText;

    ImageButton newbutton;
    GlitchEffect m_FreddyGlitchEffect;
    GlitchEffect m_StaticGlitchEffect;
    GlitchEffect m_WhiteGlitchEffect;

    enum GameplayTransitionState {
        MAIN_MENU,
        FADE_IN,
        NEWSPAPER,
        FADE_OUT,
        TIME_DISPLAY,
        LOADING_SCREEN,
        COMPLETE
    } m_GameplayTransitionState = MAIN_MENU;

    inline void SwitchState(GameplayTransitionState state);

    int m_NewsPaperTimer = 0;
    const float NEWSPAPER_DURATION = 3.0f; // 3 seconds for newspaper display
    const float TIME_DISPLAY_DURATION = 3.0f; // 3 seconds for time and night text
    const float LOADING_SCREEN_DURATION = 2.0f; // 2 seconds wait for gameplay

private:
    // Static texture collections
    std::vector<std::shared_ptr<Paingine2D::Texture>> m_NoiseTextures;
    std::vector<std::shared_ptr<Paingine2D::Texture>> m_WhiteTextures;


    std::shared_ptr<Paingine2D::Font> font = nullptr;
};
