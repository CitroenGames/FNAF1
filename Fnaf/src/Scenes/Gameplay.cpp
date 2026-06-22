#include "Gameplay.h"
#include "Paingine/Window.hpp"
#include <Paingine/Graphics.hpp>
#include "Core/Window.h"
#include "Assets/Resources.h"
#include "imgui.h"
#include "Scenes/Menu.h"
#include "GameState.h"
#include "Scene/SceneManager.h"
#include "CameraSystem.h"
#include "Graphics/ScopedView.h"
#include "Graphics/SpriteLayout.h"
#include "Utils/Helpers.h"

#include <cctype>
#include <string_view>

constexpr float m_OfficeWidth = 1600.0f;
constexpr float m_ViewportWidth = 1280.0f;

namespace {
    constexpr float GAME_VIEW_WIDTH = 1280.0f;
    constexpr float GAME_VIEW_HEIGHT = 720.0f;
    constexpr float GAME_OVER_DURATION = 2.0f;

    struct JumpscareSequenceConfig {
        const char *folder;
        const char *sound;
        float frameDuration;
        float minimumDuration;
    };

    JumpscareSequenceConfig GetJumpscareSequenceConfig(JumpscareType type) {
        switch (type) {
            case JumpscareType::Bonnie:
                return {"Graphics/JumpScares/Bonnie/", "Audio/JumpScare/AllAnimitronics.wav", 1.0f / 30.0f, 1.5f};
            case JumpscareType::Chika:
                return {"Graphics/JumpScares/Chika/", "Audio/JumpScare/AllAnimitronics.wav", 1.0f / 30.0f, 1.5f};
            case JumpscareType::Foxy:
                return {"Graphics/JumpScares/Foxy/", "Audio/JumpScare/AllAnimitronics.wav", 1.0f / 30.0f, 1.5f};
            case JumpscareType::FreddyPowerOut:
                return {"Graphics/JumpScares/FreddyPoweOut/", "Audio/JumpScare/AllAnimitronics.wav", 1.0f / 30.0f, 1.5f};
            case JumpscareType::GoldenFreddy:
                return {"Graphics/JumpScares/Gfreddy/", "Audio/JumpScare/GoldenFreddy.wav", 1.0f / 30.0f, 2.0f};
            case JumpscareType::FreddyInOffice:
            case JumpscareType::None:
            default:
                return {"Graphics/JumpScares/FreddyInOffice/", "Audio/JumpScare/AllAnimitronics.wav", 1.0f / 30.0f, 1.5f};
        }
    }

    bool TryGetFrameIndex(const std::string &filename, int &frameIndex) {
        const auto slash = filename.find_last_of("/\\");
        const std::string leaf = slash == std::string::npos ? filename : filename.substr(slash + 1);
        constexpr std::string_view prefix = "Frame";
        constexpr std::string_view extension = ".png";

        if (!leaf.starts_with(prefix) || !leaf.ends_with(extension)) {
            return false;
        }

        const std::string number = leaf.substr(prefix.size(), leaf.size() - prefix.size() - extension.size());
        if (number.empty() || !std::ranges::all_of(number, [](unsigned char c) { return std::isdigit(c); })) {
            return false;
        }

        frameIndex = std::stoi(number);
        return true;
    }

    std::vector<std::shared_ptr<Paingine2D::Texture>> LoadJumpscareFrames(const std::string &folder) {
        std::vector<std::pair<int, std::string>> frameFiles;
        for (const auto &file : Resources::ListFilesWithPrefix(folder)) {
            int frameIndex = 0;
            if (TryGetFrameIndex(file, frameIndex)) {
                frameFiles.emplace_back(frameIndex, file);
            }
        }

        std::ranges::sort(frameFiles, {}, &std::pair<int, std::string>::first);

        std::vector<std::shared_ptr<Paingine2D::Texture>> frames;
        frames.reserve(frameFiles.size());

        int expectedIndex = 0;
        for (const auto &[frameIndex, file] : frameFiles) {
            if (frameIndex != expectedIndex) {
                std::cerr << "Jumpscare sequence gap in " << folder << ": expected Frame"
                          << expectedIndex << ".png, found Frame" << frameIndex << ".png" << std::endl;
                expectedIndex = frameIndex;
            }

            if (auto texture = Resources::GetTexture(file)) {
                frames.push_back(texture);
            }
            ++expectedIndex;
        }

        if (frames.empty()) {
            std::cerr << "No jumpscare frames found in pak folder: " << folder << std::endl;
        }

        return frames;
    }

    Paingine2D::Vector2f GameViewSize() {
        return {GAME_VIEW_WIDTH, GAME_VIEW_HEIGHT};
    }
}

void Gameplay::Init() {
    gameplay = std::make_shared<FNAFGame>();
    gameplay->InitializeGame(player.m_Night);

    if (player.m_Night >= 7) // Custom night
    {
        //gameplay->InitializeCustomNight(CustomAILevels);
    }

    auto entity = CreateEntity("Office stuff");
    entity->AddComponent<Office>()->Init();
    auto officeComponent = entity->GetComponent<Office>();

    // Set the game reference in the Office component
    officeComponent->SetGameReference(gameplay);
    m_OfficeComponent = officeComponent;

    // Add camera system
    auto cameraEntity = CreateEntity("Camera System");
    cameraEntity->AddComponent<CameraSystem>()->Init();
    m_CameraSystem = cameraEntity->GetComponent<CameraSystem>();

    gameplay->SetCameraSystem(m_CameraSystem);
    m_CameraSystem->SetOfficeComponent(m_OfficeComponent);

    // HUD STUFF
    {
        // Initialize camera button (to toggle camera view) as a HUD element
        m_CameraButton = std::make_shared<HUDButton>();
        m_CameraButton->SetTexture("Graphics/CameraSystem/CameraButton.png");
        m_CameraButton->SetPosition(555.0f, 665.0f);
    }

    // Power HUD
    {
        m_PowerLeftTexture = ProcessText(Resources::GetTexture("Graphics/Gameplay/Hud/PowerLeft.png"));
        m_UsageLabelTexture = ProcessText(Resources::GetTexture("Graphics/Gameplay/Hud/Usage.png"));

        for (int i = 0; i < 5; i++) {
            m_UsageBarTextures[i] = RemoveBlackBackground(
                Resources::GetTexture("Graphics/PowerUsage/" + std::to_string(i + 1) + ".png"));
        }

        m_PowerLeftSprite.setTexture(*m_PowerLeftTexture);
        m_UsageLabelSprite.setTexture(*m_UsageLabelTexture);
        m_UsageBarsSprite.setTexture(*m_UsageBarTextures[0]);

        // Position in bottom-left
        m_PowerLeftSprite.setPosition(30.f, 590.f);
        m_UsageLabelSprite.setPosition(30.f, 620.f);

        // Usage bars positioned right after the "Usage" label
        float usageLabelRight = 30.f + m_UsageLabelSprite.getGlobalBounds().width + 8.f;
        m_UsageBarsSprite.setPosition(usageLabelRight, 620.f);

        // Power percentage text (rendered with FNAF font)
        m_PowerFont = Resources::GetFont("Font/five-nights-at-freddys.ttf");
        m_PowerPercentText.setFont(*m_PowerFont);
        m_PowerPercentText.setCharacterSize(24);
        m_PowerPercentText.setFillColor(Paingine2D::Color::White);

        // Position percentage text right after "Power left:" label
        float powerLabelRight = 30.f + m_PowerLeftSprite.getGlobalBounds().width + 8.f;
        m_PowerPercentText.setPosition(powerLabelRight, 585.f);
    }

    // Game over screen
    {
        m_GameOverBackgroundTexture = Resources::GetTexture("Graphics/Gameplay/GameOverBackground.png");
        if (m_GameOverBackgroundTexture) {
            m_GameOverBackgroundSprite.setTexture(*m_GameOverBackgroundTexture);
            SpriteLayout::FitToSizeCentered(m_GameOverBackgroundSprite, GameViewSize(), SpriteLayout::FitMode::Cover);
        }

        m_GameOverTextTexture = ProcessText(Resources::GetTexture("Graphics/Gameplay/GameOver.png"));
        if (m_GameOverTextTexture) {
            m_GameOverTextSprite.setTexture(*m_GameOverTextTexture);
            SpriteLayout::CenterAt(m_GameOverTextSprite, {GAME_VIEW_WIDTH * 0.5f, GAME_VIEW_HEIGHT * 0.5f});
        }
    }

    // Load music
    {
        bgaudio1 = Resources::GetMusic("Audio/Ambience/ambience2.wav");
        bgaudio2 = Resources::GetMusic("Audio/Ambience/EerieAmbienceLargeSca_MV005.wav");
        m_FanBuzzing = Resources::GetMusic("Audio/Office/Buzz_Fan_Florescent2.wav");

        bgaudio1->setLoop(true);
        bgaudio1->play();
        bgaudio1->setVolume(100.f);

        bgaudio2->setLoop(true);
        bgaudio2->play();
        bgaudio2->setVolume(50.f);

        m_FanBuzzing->setLoop(true);
        m_FanBuzzing->play();
        m_FanBuzzing->setVolume(40.f);
    }

    // Load sound effects? 
    {
        // for example jumpscare sound and so on.
        // IDK where else i would put this.
    }

    {
        // Camera stuff
        Camera2D::Config config;
        config.resolution = Paingine2D::Vector2f(1280.0f, 720.0f);
        config.initialZoom = 1.0f;
        config.smoothingFactor = 0.75f;
        config.maintainResolution = true;

        m_Camera = std::make_unique<Camera2D>(config);
    }
}

void Gameplay::FixedUpdate() {
    if (IsDeathSequenceActive()) {
        return;
    }

    Scene::FixedUpdate();

    // Check for main camera button press
    // Office hide/show is handled by CameraSystem in sync with the flip animation
    if (m_CameraButton->IsClicked()) {
        m_CameraSystem->ToggleCamera();
    }
}

void Gameplay::Update(double deltaTime) {
    const float frameDelta = static_cast<float>(deltaTime);

    if (IsDeathSequenceActive()) {
        UpdateDeathSequence(frameDelta);
        return;
    }

    Scene::Update(deltaTime);

    gameplay->Update(frameDelta);
    if (gameplay->IsPowerOutage() && m_FanBuzzing && m_FanBuzzing->getStatus() == AudioClip::Status::Playing) {
        m_FanBuzzing->stop();
    }

    if (gameplay->HasPendingJumpscare()) {
        StartPendingJumpscare();
        return;
    }

    if (gameplay->IsGameOver()) {
        SceneManager::QueueSwitchScene(std::make_shared<Menu>());
    }

    // Add keyboard shortcuts for debugging
    if (Paingine2D::Keyboard::isKeyPressed(Paingine2D::Keyboard::F1)) {
        // Skip to next night
        player.m_Night++;
        SceneManager::QueueSwitchScene(std::make_shared<Menu>());
    }

    if (Paingine2D::Keyboard::isKeyPressed(Paingine2D::Keyboard::F2)) {
        // Force power outage
        player.m_PowerLevel = 0;
    }

    auto window = Window::GetWindow();
    Paingine2D::Vector2i mousePos = Paingine2D::Mouse::getPosition(*window);
    Paingine2D::Vector2u windowSize = window->getSize();

    const bool cameraFeedVisible = m_CameraSystem && m_CameraSystem->IsCameraFeedVisible();
    const bool cameraTransitioning = m_CameraSystem && m_CameraSystem->IsTransitioning();

    // Handle panning for office, or camera feed once the monitor has fully covered the office.
    if (cameraFeedVisible) {
        // Camera feed panning - Camera2D at 640 (center) + pan offset
        float panOffset = m_CameraSystem->GetCameraPanOffset();
        Paingine2D::Vector2f newCameraPos(
            (m_ViewportWidth / 2.0f) + panOffset,
            720.0f / 2.0f
        );
        m_Camera->setPosition(newCameraPos);
    }
    else {
        // Scrolling parameters
        const float scrollThreshold = 400.0f;
        const float maxScrollSpeed = 500.0f;
        float scrollSpeed = 0.0f;

        // Check if mouse is actually within the window
        bool mouseInWindow = (mousePos.x >= 0 && mousePos.x < static_cast<int>(windowSize.x) &&
                              mousePos.y >= 0 && mousePos.y < static_cast<int>(windowSize.y));

        const bool canScrollOffice = !player.m_UsingCamera && !cameraTransitioning;
        if (canScrollOffice && mouseInWindow) {
            // Left edge scrolling with dynamic speed
            if (mousePos.x < scrollThreshold) {
                // Calculate how far the mouse is from the edge (0 to scrollThreshold)
                float distanceFromEdge = static_cast<float>(mousePos.x);
                // Convert to a percentage (1.0 at edge, 0.0 at threshold)
                float speedFactor = 1.0f - (distanceFromEdge / scrollThreshold);
                // Apply exponential curve for more natural acceleration
                speedFactor = speedFactor * speedFactor;
                // Apply to max speed
                scrollSpeed = -maxScrollSpeed * speedFactor;
            }
            // Right edge scrolling with dynamic speed
            else if (mousePos.x > (windowSize.x - scrollThreshold)) {
                // Calculate how far the mouse is from the edge (0 to scrollThreshold)
                float distanceFromEdge = static_cast<float>(windowSize.x - mousePos.x);
                // Convert to a percentage (1.0 at edge, 0.0 at threshold)
                float speedFactor = 1.0f - (distanceFromEdge / scrollThreshold);
                // Apply exponential curve for more natural acceleration
                speedFactor = speedFactor * speedFactor;
                // Apply to max speed
                scrollSpeed = maxScrollSpeed * speedFactor;
            }
        }

        // Update scroll offset
        scrollOffset += scrollSpeed * frameDelta;
        // Clamp scroll offset to image bounds
        scrollOffset = std::clamp(
            scrollOffset,
            0.0f,
            std::max(0.0f, m_OfficeWidth - m_ViewportWidth)
        );
        // Calculate new camera position
        Paingine2D::Vector2f newCameraPos(
            scrollOffset + (m_ViewportWidth / 2.0f), // Center horizontally
            (720.0f / 2.0f) // Center vertically
        );
        m_Camera->setPosition(newCameraPos);
    }

    // Always update the camera (needed for animations and transitions)
    m_Camera->update(frameDelta);

    // Apply camera to the render window for game elements
    m_Camera->applyTo(*window);
}

void Gameplay::Render() {
    auto window = Window::GetWindow();

    // Now draw HUD elements in screen space
    if (!IsDeathSequenceActive() && m_CameraButton && (!gameplay || !gameplay->IsPowerOutage())) {
        m_CameraButton->Draw(*window);
    }

    // Draw power HUD in screen space
    if (!IsDeathSequenceActive() && gameplay && !gameplay->IsPowerOutage()) {
        const Paingine2D::View currentView = window->getView();
        const Paingine2D::FloatRect viewport = currentView.getViewport();
        ScopedView screenView(*window, window->getDefaultView());

        // Compute viewport offset for letterboxing/pillarboxing
        Paingine2D::Vector2u winSize = window->getSize();
        auto adjustPos = [&](Paingine2D::Vector2f pos) -> Paingine2D::Vector2f {
            Paingine2D::Vector2f adjusted = pos;
            if (viewport.width < 1.0f) {
                adjusted.x = (viewport.left * winSize.x) + (pos.x * viewport.width);
            }
            if (viewport.height < 1.0f) {
                adjusted.y = (viewport.top * winSize.y) + (pos.y * viewport.height);
            }
            return adjusted;
        };

        // Update usage bars based on current usage level
        int usageIndex = std::clamp(player.m_UsageLevel, 1, 4) - 1;
        m_UsageBarsSprite.setTexture(*m_UsageBarTextures[usageIndex]);

        // Update power percentage text
        int powerPercent = std::max(0, static_cast<int>(player.m_PowerLevel));
        m_PowerPercentText.setString(std::to_string(powerPercent) + "%");

        // Adjust all positions for viewport
        m_PowerLeftSprite.setPosition(adjustPos({30.f, 590.f}));
        m_PowerPercentText.setPosition(adjustPos({30.f + m_PowerLeftSprite.getLocalBounds().width + 8.f, 585.f}));
        m_UsageLabelSprite.setPosition(adjustPos({30.f, 620.f}));
        m_UsageBarsSprite.setPosition(adjustPos({30.f + m_UsageLabelSprite.getLocalBounds().width + 8.f, 620.f}));

        window->draw(m_PowerLeftSprite);
        window->draw(m_PowerPercentText);
        window->draw(m_UsageLabelSprite);
        window->draw(m_UsageBarsSprite);

    }

    if (IsDeathSequenceActive()) {
        DrawDeathSequence(*window);
    }

#if defined(_DEBUG)
    ImGui::Begin("Jumpscare Debug"); {
        if (gameplay) {
            const bool canTrigger = !IsDeathSequenceActive() && !gameplay->HasPendingJumpscare();
            if (canTrigger) {
                if (ImGui::Button("Bonnie")) gameplay->DebugTriggerJumpscare(JumpscareType::Bonnie);
                if (ImGui::Button("Chika")) gameplay->DebugTriggerJumpscare(JumpscareType::Chika);
                if (ImGui::Button("Foxy")) gameplay->DebugTriggerJumpscare(JumpscareType::Foxy);
                if (ImGui::Button("Freddy In Office")) gameplay->DebugTriggerJumpscare(JumpscareType::FreddyInOffice);
                if (ImGui::Button("Freddy Power Outage")) gameplay->DebugTriggerJumpscare(JumpscareType::FreddyPowerOut);
                if (ImGui::Button("Golden Freddy")) gameplay->DebugTriggerJumpscare(JumpscareType::GoldenFreddy);
            } else {
                ImGui::Text("Death sequence active");
            }
        } else {
            ImGui::Text("Gameplay not initialized!");
        }
    }
    ImGui::End();

    // Render UI debug information
    ImGui::Begin("PlayerInfo"); {
        ImGui::Text("Night: %d", player.m_Night);
        ImGui::Text("Time: %d AM", (player.m_Time == 0) ? 12 : player.m_Time);
        ImGui::Text("Power Usage: %d", player.m_UsageLevel);
        ImGui::Text("Power: %d%%", static_cast<int>(player.m_PowerLevel));
    }
    ImGui::End();

    // Add new debug window for animatronic AI information
    ImGui::Begin("Animatronics AI Debug"); {
        if (gameplay) {
            // Helper function to convert Room enum to string
            auto RoomToString = [](Room room) -> std::string {
                switch (room) {
                    case Room::SHOW_STAGE: return "Show Stage";
                    case Room::DINING_AREA: return "Dining Area";
                    case Room::PIRATE_COVE: return "Pirate Cove";
                    case Room::WEST_HALL: return "West Hall";
                    case Room::EAST_HALL: return "East Hall";
                    case Room::WEST_CORNER: return "West Hall Corner";
                    case Room::EAST_CORNER: return "East Hall Corner";
                    case Room::SUPPLY_CLOSET: return "Supply Closet";
                    case Room::KITCHEN: return "Kitchen";
                    case Room::RESTROOMS: return "Restrooms";
                    case Room::OFFICE: return "Office";
                    default: return "Unknown";
                }
            };

            std::vector<std::string> characters = {"Freddy", "Bonnie", "Chica", "Foxy"};

            for (const auto &character: characters) {
                if (ImGui::CollapsingHeader(character.c_str())) {
                    // Get animatronic information
                    Room location = gameplay->GetAnimatronicLocation(character);
                    float progress = gameplay->GetAnimatronicMovementProgress(character);

                    // Display room location
                    ImGui::Text("Location: %s", RoomToString(location).c_str());

                    // Get AI info from gameplay - need to expose this in the FNAFGame class
                    int aiLevel = gameplay->GetAnimatronicAILevel(character);
                    float timeSinceLastMove = gameplay->GetAnimatronicTimeSinceLastMove(character);
                    float moveInterval = gameplay->GetAnimatronicMoveInterval(character);

                    ImGui::Text("AI Level: %d / %d", aiLevel, MAX_AI_LEVEL);
                    ImGui::Text("Movement Progress: %.1f%%", progress);
                    ImGui::Text("Move Timer: %.2fs / %.2fs", timeSinceLastMove, moveInterval);

                    // Add progress bar for move timer
                    float timerRatio = timeSinceLastMove / moveInterval;
                    ImGui::ProgressBar(timerRatio, ImVec2(-1, 0),
                                       ("Move Chance: " + std::to_string(int(100 * std::min(1.0f,
                                                                                 float(aiLevel) /
                                                                                 float(MAX_RANDOM_ROLL)))) + "%").
                                       c_str());

                    // Add a visual indicator for movement probability
                    ImGui::Text("Movement Roll: AI Level (%d) vs Random(1-20)", aiLevel);

                    // Character-specific info
                    if (character == "Freddy") {
                        ImGui::Text("Special: Stalled by camera, moves in dark");
                    } else if (character == "Bonnie") {
                        ImGui::Text("Special: Approaches from left side");
                    } else if (character == "Chica") {
                        ImGui::Text("Special: Approaches from right side, makes kitchen noises");
                    } else if (character == "Foxy") {
                        ImGui::Text("Special: Simplified behavior (original complex AI commented out)");
                    }
                }
            }
        } else {
            ImGui::Text("Gameplay not initialized!");
        }
    }
    ImGui::End();

    // New debug window for power system
    ImGui::Begin("Power System Debug"); {
        if (gameplay) {
            ImGui::Text("Power Level: %.2f%%", player.m_PowerLevel);

            // Display active systems using new centralized state
            ImGui::Text("Active Systems:");
            ImGui::Text("- Camera: %s", player.m_UsingCamera ? "ON" : "OFF");
            ImGui::Text("- Left Door: %s", player.m_LeftDoorClosed ? "CLOSED" : "OPEN");
            ImGui::Text("- Right Door: %s", player.m_RightDoorClosed ? "CLOSED" : "OPEN");
            ImGui::Text("- Left Light: %s", player.m_LeftLightOn ? "ON" : "OFF");
            ImGui::Text("- Right Light: %s", player.m_RightLightOn ? "ON" : "OFF");

            // Display usage level and drain rate
            ImGui::Text("Usage Level: %d/5", player.m_UsageLevel);
            ImGui::Text("Calculated Usage: %d/5", player.CalculateUsageLevel());

            // Display power outage info if applicable
            if (gameplay->IsPowerOutage()) {
                ImGui::Separator();
                const char* phaseNames[] = { "DARK_WAIT", "FREDDY_FACE", "LIGHTS_OFF", "JUMPSCARE" };
                ImGui::Text("POWER OUTAGE - Phase: %s", phaseNames[static_cast<int>(gameplay->GetPowerOutagePhase())]);
                ImGui::Text("Phase Timer: %.2f seconds", gameplay->GetPhaseTimer());
            }
        } else {
            ImGui::Text("Gameplay not initialized!");
        }
    }
    ImGui::End();
#endif
}

void Gameplay::Destroy() {
    if (m_JumpscareSound) {
        m_JumpscareSound->stop();
        m_JumpscareSound.reset();
    }

    if (m_CameraSystem) {
        m_CameraSystem->Destroy();
    }

    if (m_OfficeComponent) {
        m_OfficeComponent->Destroy();
    }
    Scene::Destroy();
}

void Gameplay::StartPendingJumpscare() {
    if (!gameplay || !gameplay->HasPendingJumpscare()) {
        return;
    }

    const JumpscareType type = gameplay->GetPendingJumpscare();
    gameplay->ClearPendingJumpscare();
    StartJumpscare(type);
}

void Gameplay::StartJumpscare(JumpscareType type) {
    const JumpscareSequenceConfig config = GetJumpscareSequenceConfig(type);

    if (m_CameraSystem) {
        m_CameraSystem->ForceClose();
    }

    if (bgaudio1) bgaudio1->stop();
    if (bgaudio2) bgaudio2->stop();
    if (m_FanBuzzing) m_FanBuzzing->stop();

    player.m_UsingCamera = false;
    player.m_LeftLightOn = false;
    player.m_RightLightOn = false;
    player.UpdateUsageLevel();

    m_ActiveJumpscare = type;
    m_JumpscareFrames = LoadJumpscareFrames(config.folder);
    m_JumpscareFrameTimer = 0.0f;
    m_JumpscareTotalTimer = 0.0f;
    m_JumpscareFrameDuration = config.frameDuration;
    m_JumpscareMinimumDuration = config.minimumDuration;
    m_JumpscareFrameIndex = 0;
    m_DiscardNextJumpscareDelta = true;

    m_JumpscareSound = Resources::GetMusic(config.sound);
    if (m_JumpscareSound) {
        m_JumpscareSound->setLoop(false);
        m_JumpscareSound->setVolume(100.0f);
        m_JumpscareSound->play();
    }

    if (m_JumpscareFrames.empty()) {
        SwitchToGameOver();
        return;
    }

    m_JumpscareSprite.setTexture(*m_JumpscareFrames.front(), true);
    SpriteLayout::FitToSizeCentered(m_JumpscareSprite, GameViewSize(), SpriteLayout::FitMode::Cover);
    m_DeathSequenceState = DeathSequenceState::Jumpscare;
}

void Gameplay::UpdateDeathSequence(float deltaTime) {
    if (m_DeathSequenceState == DeathSequenceState::Jumpscare) {
        if (m_DiscardNextJumpscareDelta) {
            m_DiscardNextJumpscareDelta = false;
            return;
        }

        const float animationDelta = std::min(deltaTime, m_JumpscareFrameDuration);
        m_JumpscareTotalTimer += animationDelta;
        m_JumpscareFrameTimer += animationDelta;

        while (m_JumpscareFrameTimer >= m_JumpscareFrameDuration && !m_JumpscareFrames.empty()) {
            m_JumpscareFrameTimer -= m_JumpscareFrameDuration;
            m_JumpscareFrameIndex = (m_JumpscareFrameIndex + 1) % m_JumpscareFrames.size();
            m_JumpscareSprite.setTexture(*m_JumpscareFrames[m_JumpscareFrameIndex], true);
            SpriteLayout::FitToSizeCentered(m_JumpscareSprite, GameViewSize(), SpriteLayout::FitMode::Cover);
        }

        if (m_JumpscareTotalTimer >= m_JumpscareMinimumDuration) {
            if (m_ActiveJumpscare == JumpscareType::GoldenFreddy) {
                Window::GetWindow()->close();
                return;
            }

            SwitchToGameOver();
        }
        return;
    }

    if (m_DeathSequenceState == DeathSequenceState::GameOver) {
        m_GameOverTimer += deltaTime;
        if (m_GameOverTimer >= GAME_OVER_DURATION) {
            SceneManager::QueueSwitchScene(std::make_shared<Menu>());
        }
    }
}

void Gameplay::SwitchToGameOver() {
    m_DeathSequenceState = DeathSequenceState::GameOver;
    m_GameOverTimer = 0.0f;
    m_JumpscareFrames.clear();
    m_ActiveJumpscare = JumpscareType::None;
    m_DiscardNextJumpscareDelta = false;
}

void Gameplay::DrawDeathSequence(Paingine2D::RenderWindow &window) {
    ScopedView screenView(window, window.getDefaultView());

    if (m_DeathSequenceState == DeathSequenceState::Jumpscare && m_JumpscareFrames.size() > 0) {
        window.draw(m_JumpscareSprite);
    } else if (m_DeathSequenceState == DeathSequenceState::GameOver) {
        if (m_GameOverBackgroundTexture) {
            window.draw(m_GameOverBackgroundSprite);
        }
        if (m_GameOverTextTexture) {
            window.draw(m_GameOverTextSprite);
        }
    }
}
