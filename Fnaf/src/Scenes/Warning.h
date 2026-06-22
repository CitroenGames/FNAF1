#pragma once

#include "Scene/Scene.h"

class WarningMessage : public Scene {
public:
    WarningMessage();

    void Init() override;

    void Update(double deltaTime) override;

    void FixedUpdate() override {
    };

    void Render() override {
    };

    void Destroy() override {
    };

    void SwitchToMainMenu();

private:
    std::shared_ptr<Paingine2D::Texture> m_WarningMessageTexture;
    Paingine2D::Sprite m_WarningMessageSprite;

    enum State { WARNING, SWITCHING, MAIN_MENU } m_State = WARNING;

    float m_WarningMessageTimer = 0.0f;
};
