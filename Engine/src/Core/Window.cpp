#include "Window.h"

#include <algorithm>
#include <stdexcept>

#include <Paingine/Graphics/View.hpp>
#include <Paingine/Window/VideoMode.hpp>

namespace {
    std::shared_ptr<Paingine2D::RenderWindow> g_Window;
    Paingine2D::View g_GameView;
    Window::Config g_Config;

    Paingine2D::Vector2f ValidDesignResolution(Paingine2D::Vector2f resolution) {
        if (resolution.x <= 0.0f || resolution.y <= 0.0f) {
            throw std::invalid_argument("Window design resolution must be positive");
        }

        return resolution;
    }
}

std::shared_ptr<Paingine2D::RenderWindow> Window::Init(int width, int height, const std::string &title) {
    Config config;
    config.width = width;
    config.height = height;
    config.title = title;
    config.designResolution = {
        static_cast<float>(std::max(width, 1)),
        static_cast<float>(std::max(height, 1))
    };
    return Init(config);
}

std::shared_ptr<Paingine2D::RenderWindow> Window::Init(const Config &config) {
    if (g_Window) {
        return g_Window;
    }

    g_Config = config;
    g_Config.width = std::max(g_Config.width, 1);
    g_Config.height = std::max(g_Config.height, 1);
    g_Config.designResolution = ValidDesignResolution(g_Config.designResolution);

    g_Window = std::make_shared<Paingine2D::RenderWindow>(
        Paingine2D::VideoMode(static_cast<unsigned int>(g_Config.width), static_cast<unsigned int>(g_Config.height)),
        g_Config.title);
    g_GameView = GetDesignView();
    g_Window->setView(g_GameView);
    UpdateViewport();
    return g_Window;
}

void Window::UpdateViewport() {
    if (!g_Window) {
        return;
    }

    const Paingine2D::Vector2u windowSize = g_Window->getSize();
    if (windowSize.x == 0 || windowSize.y == 0) {
        return;
    }

    const Paingine2D::Vector2f designResolution = GetDesignResolution();
    const float windowAspectRatio = static_cast<float>(windowSize.x) / static_cast<float>(windowSize.y);
    const float viewportAspectRatio = designResolution.x / designResolution.y;

    g_GameView.setViewport(Paingine2D::FloatRect(0.0f, 0.0f, 1.0f, 1.0f));

    if (g_Config.scaleMode == ScaleMode::Stretch) {
        g_GameView.setSize(designResolution);
        g_Window->setView(g_GameView);
        return;
    }

    if (g_Config.scaleMode == ScaleMode::Letterbox) {
        Paingine2D::FloatRect viewport(0.0f, 0.0f, 1.0f, 1.0f);
        if (windowAspectRatio > viewportAspectRatio) {
            const float width = viewportAspectRatio / windowAspectRatio;
            viewport.left = (1.0f - width) * 0.5f;
            viewport.width = width;
        } else if (windowAspectRatio < viewportAspectRatio) {
            const float height = windowAspectRatio / viewportAspectRatio;
            viewport.top = (1.0f - height) * 0.5f;
            viewport.height = height;
        }

        g_GameView.setSize(designResolution);
        g_GameView.setViewport(viewport);
        g_Window->setView(g_GameView);
        return;
    }

    float viewWidth = designResolution.x;
    float viewHeight = designResolution.y;

    if (windowAspectRatio > viewportAspectRatio) {
        viewWidth = viewHeight * windowAspectRatio;
    } else {
        viewHeight = viewWidth / windowAspectRatio;
    }

    const Paingine2D::Vector2f currentCenter = g_GameView.getCenter();
    g_GameView.setSize(viewWidth, viewHeight);
    g_GameView.setCenter(currentCenter);
    g_Window->setView(g_GameView);
}

void Window::Destroy() {
    if (!g_Window) {
        return;
    }

    if (g_Window->isOpen()) {
        g_Window->close();
    }

    g_Window.reset();
}

std::shared_ptr<Paingine2D::RenderWindow> Window::GetWindow() {
    return g_Window;
}

const Window::Config &Window::GetConfig() {
    return g_Config;
}

Paingine2D::Vector2f Window::GetDesignResolution() {
    return g_Config.designResolution;
}

Window::ScaleMode Window::GetScaleMode() {
    return g_Config.scaleMode;
}

void Window::SetDesignResolution(Paingine2D::Vector2f resolution) {
    g_Config.designResolution = ValidDesignResolution(resolution);
    const Paingine2D::Vector2f currentCenter = g_GameView.getCenter();
    g_GameView = GetDesignView();
    g_GameView.setCenter(currentCenter);
    UpdateViewport();
}

void Window::SetScaleMode(ScaleMode scaleMode) {
    g_Config.scaleMode = scaleMode;
    UpdateViewport();
}

Paingine2D::View Window::GetDesignView() {
    const Paingine2D::Vector2f designResolution = GetDesignResolution();
    Paingine2D::View view;
    view.setSize(designResolution);
    view.setCenter(designResolution.x * 0.5f, designResolution.y * 0.5f);
    return view;
}
