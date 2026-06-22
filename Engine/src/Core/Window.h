#pragma once

#include <memory>
#include <string>

#include <Paingine/Graphics/RenderWindow.hpp>
#include <Paingine/Graphics/View.hpp>
#include <Paingine/System/Vector2.hpp>

constexpr float VIEWPORT_WIDTH = 1280.0f;
constexpr float VIEWPORT_HEIGHT = 720.0f;

class Window {
public:
    enum class ScaleMode {
        Expand,
        Letterbox,
        Stretch
    };

    struct Config {
        int width = 1280;
        int height = 720;
        std::string title = "Window";
        Paingine2D::Vector2f designResolution = {VIEWPORT_WIDTH, VIEWPORT_HEIGHT};
        ScaleMode scaleMode = ScaleMode::Expand;
    };

    static std::shared_ptr<Paingine2D::RenderWindow> Init(int width, int height, const std::string &title);
    static std::shared_ptr<Paingine2D::RenderWindow> Init(const Config &config);

    static void UpdateViewport();

    static void Destroy();

    static std::shared_ptr<Paingine2D::RenderWindow> GetWindow();

    static const Config &GetConfig();
    static Paingine2D::Vector2f GetDesignResolution();
    static ScaleMode GetScaleMode();
    static void SetDesignResolution(Paingine2D::Vector2f resolution);
    static void SetScaleMode(ScaleMode scaleMode);
    static Paingine2D::View GetDesignView();
};
