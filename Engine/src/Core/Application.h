#pragma once

#include <string>

#include <Paingine/Graphics/Color.hpp>

#include "Core/Window.h"

class Application {
public:
    // Fixed-update rate used when a config leaves it unset or supplies a non-positive value.
    static constexpr int DEFAULT_FIXED_TICK_RATE = 66;

    struct Config {
        Window::Config window;
        int fixedTickRate = DEFAULT_FIXED_TICK_RATE;
        bool verticalSync = true;
        bool enableImGui = true;
        Paingine2D::Color clearColor = Paingine2D::Color::Black;
    };

    static void Init(int width = 1280, int height = 720, const std::string &title = "Window");
    static void Init(const Config &config);
    static Config NativeResolutionConfig(
        int width,
        int height,
        const std::string &title,
        int fixedTickRate = 60);

    static void Run();

    static void Destroy();

    static const Config &GetConfig();

    // Seconds advanced by one SceneManager::FixedUpdate. Prefer this over
    // hardcoding the tick rate so systems stay correct if the config changes.
    static float GetFixedDeltaTime();
};
