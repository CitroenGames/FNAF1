#pragma once

#include "Animation/FlipBook.h"
#include "Audio/AudioClip.h"
#include "UI/HUDButton.h"
#include "Composable.h"
#include "UI/ImageButton.h"

class Office;

class CameraSystem : public Composable::Component {
public:
    CameraSystem();
    ~CameraSystem() = default;

    void Init();
    void Update(double deltaTime) override;
    void FixedUpdate() override;
    void Destroy();

    void ToggleCamera();
    void ForceClose();
    void SetActiveCamera(const std::string& cameraId);
    bool IsActive() const { return m_IsActive; }
    bool IsTransitioning() const { return m_IsAnimatingOpen || m_IsAnimatingClose; }
    bool IsCameraFeedVisible() const { return m_IsActive && !m_IsAnimatingOpen; }
    void SetOfficeComponent(std::shared_ptr<Office> office) { m_OfficeRef = office; }
    float GetCameraPanOffset() const;
    
    nlohmann::json Serialize() const override { return nlohmann::json(); }
    void Deserialize(const nlohmann::json& data) override {}
    std::string GetTypeName() const override { return "CameraSystem"; }
    std::string GetActiveCamera() const { return m_CurrentCamera; }

private:
    bool m_IsActive;
    bool m_IsAnimatingOpen = false;
    bool m_IsAnimatingClose = false;
    std::string m_CurrentCamera;

    // Camera views
    std::map<std::string, std::shared_ptr<Paingine2D::Sprite>> m_CameraViews;
    std::map<std::string, std::map<std::string, std::shared_ptr<Paingine2D::Sprite>>> m_CameraStateViews;

    // UI elements
    std::shared_ptr<Paingine2D::Sprite> m_CameraMapSprite;
    std::shared_ptr<Paingine2D::Sprite> m_CameraBorderSprite;
    std::map<std::string, std::shared_ptr<ImageButton>> m_CameraButtons;

    // Camera name text overlays
    std::map<std::string, std::shared_ptr<Paingine2D::Sprite>> m_CameraNameSprites;

    // Animation elements
    FlipBook m_CameraFlipAnimation;
    FlipBook m_StaticAnimation;

    // Camera feed panning (mouse-based, like office scrolling)
    float m_CameraPanOffset = 0.0f;  // 0=center, negative=left, positive=right
    float m_PanDirection = 1.0f;       // 1.0=right, -1.0=left
    float m_PanPauseTimer = 0.0f;      // countdown at edges before reversing

    // Base screen positions for UI elements (repositioned when panning)
    Paingine2D::Vector2f m_MapBasePos = {1012.0f, 515.0f};
    Paingine2D::Vector2f m_BorderBasePos = {640.0f, 360.0f};
    std::map<std::string, Paingine2D::Vector2f> m_ButtonBasePositions;
    std::map<std::string, Paingine2D::Vector2f> m_NameBasePositions;

    void InitializeCameraViews();
    void InitializeCameraButtons();
    void InitializeAnimations();
    void InitializeCameraOverlays();
    void ShowCameraView(const std::string& cameraId);
    void ShowCameraOverlays();
    void HideCameraOverlays();
    void UpdateCameraViewBasedOnAnimatronics();
    void HideAllCameraElements();
    void UpdateCameraMovement(double deltaTime);
    void UpdateUIPositions();
    bool IsCurrentCameraPannable() const;
    
    // Office reference for syncing visibility with animation
    std::shared_ptr<Office> m_OfficeRef;

    // Sound effects
    std::shared_ptr<AudioClip> m_CameraOpenSound;
    std::shared_ptr<AudioClip> m_CameraCloseSound;
    std::shared_ptr<AudioClip> m_CameraSwitchSound;
    std::shared_ptr<AudioClip> m_CameraMonitorUpSound;
};
