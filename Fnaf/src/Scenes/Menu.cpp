#include "Menu.h"
#include "Scene/SceneManager.h"
#include "Scenes/Gameplay.h"
#include "Assets/Resources.h"
#include "Core/Window.h"
#include "Graphics/LayerManager.h"
#include "LayerDefines.h"
#include "Utils/Helpers.h"
#include "Audio/AudioManager.h"
#include "UI/Layout.h"

namespace {
    constexpr float FIXED_TICK_RATE = 66.0f;
}

Menu::Menu() {
    // Preload audio - no longer keeping music objects directly in the Menu class
    Resources::GetMusic(STATIC_AUDIO_KEY);
    Resources::GetMusic(MENU_MUSIC_KEY);

    // Noise textures
    for (int i = 1; i <= 8; i++) {
        m_NoiseTextures.push_back(MakeTextureTransparent(
            Resources::GetTexture("Graphics/Static/Noise" + std::to_string(i) + ".png"),
            0.15f
        ));
    }

    // Load & Setup white textures
    for (int i = 1; i <= 8; i++) {
        m_WhiteTextures.push_back(RemoveBlackBackground(
            Resources::GetTexture("Graphics/White/WhiteThing" + std::to_string(i) + ".png")
        ));
    }

    // Load & Setup Freddy glitch effect
    m_FreddyGlitchEffect = GlitchEffect(0);
    for (int i = 1; i <= 4; i++) {
        m_FreddyGlitchEffect.AddFrame(Resources::GetTexture(
            "Graphics/MainMenu/FreddyBackground/Frame" + std::to_string(i) + ".png"
        ));
    }

    // Load textures
    m_Logo = ProcessText(Resources::GetTexture("Graphics/MainMenu/Logo.png"));
    NewsPaperTexture = Resources::GetTexture("Graphics/MainMenu/NewsPaper.png");
    m_WarningMessageTexture = ProcessText(Resources::GetTexture("Graphics/MainMenu/WarningMessage.png"));

    // Setup warning message sprite
    m_WarningMessageSprite.setTexture(*m_WarningMessageTexture);
    const Paingine2D::Vector2u windowSize = Window::GetWindow()->getSize();
    m_WarningMessageSprite.setPosition(UI::Layout::CenteredPosition(windowSize, m_WarningMessageSprite.getGlobalBounds()));

    // load loading screen sprite
    m_LoadingScreenTexture = Resources::GetTexture("Graphics/Loading.png");

    // load time and night text
    font = Resources::GetFont("Font/five-nights-at-freddys.ttf");

    // Prepare logo sprite
    m_LogoSprite = Paingine2D::Sprite(*m_Logo);
    m_LogoSprite.setPosition(100, 100);
}

void Menu::Init() {
    m_NewsPaperTimer = 0;
    m_GameplayTransitionState = MAIN_MENU;
    ShowMainMenuElements();

    // Start audio using AudioManager
    AudioManager::GetInstance().PlayMusic(STATIC_AUDIO_KEY, false, 100.0f);
    AudioManager::GetInstance().PlayMusic(MENU_MUSIC_KEY, true, 100.0f);

    m_TimeText.setFont(*font);
    m_TimeText.setString("12:00 AM");
    m_TimeText.setCharacterSize(75);
    m_TimeText.setFillColor(Paingine2D::Color::White);
    const Paingine2D::Vector2u windowSize = Window::GetWindow()->getSize();
    m_TimeText.setPosition(
        UI::Layout::CenteredX(windowSize, m_TimeText.getGlobalBounds()),
        static_cast<float>(windowSize.y) / 2.0f - 25.0f
    );

    m_NightText.setFont(*font);
    m_NightText.setString("1st Night");
    m_NightText.setCharacterSize(75);
    m_NightText.setFillColor(Paingine2D::Color::White);
    m_NightText.setPosition(
        UI::Layout::CenteredX(windowSize, m_NightText.getGlobalBounds()),
        static_cast<float>(windowSize.y) / 2.0f + 25.0f
    );

    // Setup loading screen sprite
    m_LoadingScreenSprite.setTexture(*m_LoadingScreenTexture);
    m_LoadingScreenSprite.setPosition(
        static_cast<float>(windowSize.x) - m_LoadingScreenSprite.getGlobalBounds().width,
        static_cast<float>(windowSize.y) - m_LoadingScreenSprite.getGlobalBounds().height
    );

    // Setup white glitch effect
    m_WhiteGlitchEffect = GlitchEffect(2);
    for (int i = 0; i < m_WhiteTextures.size(); i++) {
        m_WhiteGlitchEffect.AddFrame(m_WhiteTextures[i]);
    }

    // Setup static glitch effect
    m_StaticGlitchEffect = GlitchEffect(1);
    for (int i = 0; i < m_NoiseTextures.size(); i++) {
        m_StaticGlitchEffect.AddFrame(m_NoiseTextures[i]);
    }

    // Configure glitch parameters
    m_FreddyGlitchEffect.SetGlitchParameters(0.01f, 0.3f, 0.05f);
    m_StaticGlitchEffect.SetGlitchParameters(1.f, 1.5f, 0.01f);
    m_WhiteGlitchEffect.SetGlitchParameters(1.f, 1.5f, 0.15f);

    // Prepare newspaper sprite
    NewsPaperSprite = Paingine2D::Sprite(*NewsPaperTexture);
    NewsPaperSprite.setPosition(UI::Layout::CenteredPosition(windowSize, NewsPaperSprite.getGlobalBounds()));

    // Setup new game button
    newbutton.SetTexture(ProcessText(Resources::GetTexture("Graphics/MainMenu/NewGame.png")));
    newbutton.SetPosition(100, 400);
}

void Menu::HideGlitchEffects() {
    // Set glitch effects to an invisible layer
    m_FreddyGlitchEffect.SetLayer(-1);
    m_FreddyGlitchEffect.Kill();
    m_StaticGlitchEffect.SetLayer(-1);
    m_StaticGlitchEffect.Kill();
    m_WhiteGlitchEffect.SetLayer(-1);
    m_WhiteGlitchEffect.Kill();
}

void Menu::ShowGlitchEffects() {
    // Restore glitch effects to their original layers
    m_FreddyGlitchEffect.SetLayer(0);
    m_StaticGlitchEffect.SetLayer(1);
    m_WhiteGlitchEffect.SetLayer(2);
}

void Menu::SwitchState(GameplayTransitionState state) {
    m_GameplayTransitionState = state;
    m_NewsPaperTimer = 0; // Reset timer
}

void Menu::HideAllMenuElements() {
    // Remove all menu elements from the layer manager
    LayerManager::RemoveDrawable(&m_LogoSprite);
    LayerManager::RemoveDrawable(&NewsPaperSprite);
    LayerManager::RemoveDrawable(&m_TimeText);
    LayerManager::RemoveDrawable(&m_NightText);
    LayerManager::RemoveDrawable(&m_LoadingScreenSprite);
    LayerManager::RemoveDrawable(&m_WarningMessageSprite);

    // Remove buttons from layer
    newbutton.SetVisible(false);

    // Hide glitch effects
    HideGlitchEffects();
}

void Menu::ShowMainMenuElements() {
    // Add main menu elements to the layer manager
    LayerManager::AddDrawable(MENU_BUTTON_LAYER, &m_LogoSprite);

    // Add new game button to the visible layer
    newbutton.SetLayer(MENU_BUTTON_LAYER);

    // Show glitch effects
    ShowGlitchEffects();
}

void Menu::Update(double deltaTime) {
    (void) deltaTime;
}

void Menu::SwitchToGameplay() {
    // Clean up all transition elements before switching scenes
    HideAllMenuElements();

    // Switch to gameplay scene
    SceneManager::QueueSwitchScene(std::make_shared<Gameplay>());
}

void Menu::FixedUpdate() {
    m_NewsPaperTimer += 1;
    const float seconds = static_cast<float>(m_NewsPaperTimer) / FIXED_TICK_RATE;
    switch (m_GameplayTransitionState) {
        case MAIN_MENU: {
            m_FreddyGlitchEffect.Update();
            m_StaticGlitchEffect.Update();
            m_WhiteGlitchEffect.Update();

            if (newbutton.IsClicked()) {
                // Hide all menu elements when transitioning to newspaper
                HideAllMenuElements();

                // Show only the newspaper
                LayerManager::AddDrawable(MENU_BUTTON_LAYER, &NewsPaperSprite);
                SwitchState(NEWSPAPER);
            }
            break;
        }
        case FADE_IN: {
            break;
        }
        case NEWSPAPER: {
            if (seconds >= NEWSPAPER_DURATION) {
                // Switch to time display
                LayerManager::RemoveDrawable(&NewsPaperSprite);
                LayerManager::AddDrawable(MENU_BUTTON_LAYER, &m_TimeText);
                LayerManager::AddDrawable(MENU_BUTTON_LAYER, &m_NightText);
                SwitchState(TIME_DISPLAY);
            }
            break;
        }

        case FADE_OUT: {
            break;
        }

        case TIME_DISPLAY: {
            if (seconds >= TIME_DISPLAY_DURATION) {
                // Switch to loading screen
                LayerManager::RemoveDrawable(&m_TimeText);
                LayerManager::RemoveDrawable(&m_NightText);
                LayerManager::AddDrawable(MENU_BUTTON_LAYER, &m_LoadingScreenSprite);
                SwitchState(LOADING_SCREEN);
            }
            break;
        }

        case LOADING_SCREEN: {
            if (seconds >= LOADING_SCREEN_DURATION) {
                // Transition complete, switch to gameplay
                SwitchState(COMPLETE);
                SwitchToGameplay();
            }
            break;
        }


        default:
            break;
    }
}
