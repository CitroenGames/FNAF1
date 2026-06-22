#include "Warning.h"
#include "Scene/SceneManager.h"
#include "Assets/Resources.h"
#include "Graphics/LayerManager.h"
#include "Core/Window.h"
#include "LayerDefines.h"
#include "Utils/Helpers.h"
#include "Scenes/Menu.h"
#include "UI/Layout.h"

namespace {
    std::shared_ptr<Menu> MainMenuScene;

#if _DEBUG
    constexpr float WARNING_MESSAGE_DURATION = 2.0f;
#else
    constexpr float WARNING_MESSAGE_DURATION = 5.0f;
#endif

}

WarningMessage::WarningMessage() {
    // Load warning message texture
    m_WarningMessageTexture = ProcessText(Resources::GetTexture("Graphics/MainMenu/WarningMessage.png"));
    m_WarningMessageSprite.setTexture(*m_WarningMessageTexture);

    // Center the warning message
    const Paingine2D::Vector2u windowSize = Window::GetWindow()->getSize();
    m_WarningMessageSprite.setPosition(UI::Layout::CenteredPosition(windowSize, m_WarningMessageSprite.getGlobalBounds()));

    // reason why we are doing this is to not waste time on loading the menu while we wait for the warning message
    MainMenuScene = std::make_shared<Menu>();
}

void WarningMessage::Init() {
    // Start with warning message
    m_State = WARNING;
    m_WarningMessageTimer = 0.0f;
    LayerManager::AddDrawable(MENU_BUTTON_LAYER, &m_WarningMessageSprite);
}

void WarningMessage::Update(double deltaTime) {
    if (m_State == WARNING) {
        m_WarningMessageTimer += static_cast<float>(deltaTime);
        if (m_WarningMessageTimer >= WARNING_MESSAGE_DURATION) {
            // Set state to switching to indicate we're ready to switch scenes
            m_State = SWITCHING;
            SwitchToMainMenu();
        }
    }
}

void WarningMessage::SwitchToMainMenu() {
    // Remove warning message from layer
    LayerManager::RemoveDrawable(&m_WarningMessageSprite);

    // Clean up before switching scenes
    Destroy();

    // Switch to Menu scene
    SceneManager::QueueSwitchScene(MainMenuScene);
}
