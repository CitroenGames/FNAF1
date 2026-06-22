#pragma once

#include <cstddef>
#include <utility>
#include <vector>

#include <Paingine/Graphics/Rect.hpp>
#include <Paingine/Graphics/RenderTarget.hpp>
#include <Paingine/Graphics/View.hpp>
#include <Paingine/System/Vector2.hpp>

class Camera2D {
public:
    struct Config {
        Paingine2D::Vector2f center;
        Paingine2D::Vector2f resolution;
        float initialZoom;
        float minZoom;
        float maxZoom;
        float smoothingFactor;
        bool maintainResolution;

        Config();
        explicit Config(Paingine2D::Vector2f customCenter,
                        Paingine2D::Vector2f customResolution = Paingine2D::Vector2f(800.0f, 600.0f));
    };

    explicit Camera2D(const Config &config = Config{});

    Camera2D(const Camera2D &) = delete;
    Camera2D &operator=(const Camera2D &) = delete;
    Camera2D(Camera2D &&) noexcept = default;
    Camera2D &operator=(Camera2D &&) noexcept = default;
    ~Camera2D() = default;

    void setPosition(const Paingine2D::Vector2f &newPosition);
    void move(const Paingine2D::Vector2f &offset) noexcept;
    void setZoom(float zoom);
    void zoom(float factor) noexcept;
    void setRotation(float angle) noexcept;
    void rotate(float angle) noexcept;
    void focusOn(const Paingine2D::Vector2f &target, float duration = 1.0f);

    void setBounds(const Paingine2D::FloatRect &newBounds);
    void clearBounds() noexcept;

    void setMaintainResolution(bool maintain) noexcept;
    void setBaseResolution(const Paingine2D::Vector2f &resolution);

    void update(float deltaTime);
    void applyTo(Paingine2D::RenderTarget &target);

    [[nodiscard]] Paingine2D::Vector2f screenToWorldCoords(const Paingine2D::Vector2f &screenPos,
                                                   const Paingine2D::RenderTarget &target) const;
    [[nodiscard]] Paingine2D::Vector2f worldToScreenCoords(const Paingine2D::Vector2f &worldPos,
                                                   const Paingine2D::RenderTarget &target) const;

    [[nodiscard]] const Paingine2D::Vector2f &getPosition() const noexcept;
    [[nodiscard]] float getZoomLevel() const noexcept;
    [[nodiscard]] float getRotation() const noexcept;
    [[nodiscard]] Paingine2D::FloatRect getViewport() const noexcept;
    [[nodiscard]] const Paingine2D::Vector2f &getBaseResolution() const noexcept;
    [[nodiscard]] bool isMaintainingResolution() const noexcept;
    [[nodiscard]] const std::vector<std::pair<float, Paingine2D::Vector2f> > &getMotionTrail() const noexcept;
    [[nodiscard]] Paingine2D::FloatRect getVisibleArea() const noexcept;

private:
    Paingine2D::View view;
    Paingine2D::Vector2f position;
    Paingine2D::Vector2f baseResolution;
    float zoomLevel;
    float rotation;
    bool maintainResolution;
    float minZoom;
    float maxZoom;
    Paingine2D::FloatRect bounds;
    bool boundingEnabled;

    Paingine2D::Vector2f targetPosition;
    float smoothingFactor;

    float lastDeltaTime;
    std::vector<std::pair<float, Paingine2D::Vector2f> > positionHistory;
    static constexpr size_t MAX_HISTORY_SIZE = 10;

    void validateConfig(const Config &config);
    void updateView() noexcept;
    void updateViewport(const Paingine2D::Vector2u &actualWindowSize) noexcept;
    void updateViewport() noexcept;
    void setTargetPosition(const Paingine2D::Vector2f &target) noexcept;
    [[nodiscard]] Paingine2D::Vector2f clampToBounds(const Paingine2D::Vector2f &pos) const noexcept;
    void updateSmoothMovement(float deltaTime) noexcept;
    void updatePositionHistory();
    [[nodiscard]] float calculateSmoothingForSpeed(float dx, float dy) const noexcept;
};
