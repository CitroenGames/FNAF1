#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <SDL3/SDL_events.h>

struct SDL_Renderer;
struct SDL_Texture;
struct SDL_Window;

namespace Paingine2D {
using Uint8 = std::uint8_t;

template <typename T>
struct Vector2 {
    T x{};
    T y{};

    constexpr Vector2() = default;
    constexpr Vector2(T xValue, T yValue) : x(xValue), y(yValue) {}

    template <typename U>
    explicit constexpr Vector2(const Vector2<U> &other)
        : x(static_cast<T>(other.x)), y(static_cast<T>(other.y)) {}
};

using Vector2f = Vector2<float>;
using Vector2i = Vector2<int>;
using Vector2u = Vector2<unsigned int>;

template <typename T>
constexpr Vector2<T> operator+(const Vector2<T> &lhs, const Vector2<T> &rhs) {
    return {lhs.x + rhs.x, lhs.y + rhs.y};
}

template <typename T>
constexpr Vector2<T> operator-(const Vector2<T> &lhs, const Vector2<T> &rhs) {
    return {lhs.x - rhs.x, lhs.y - rhs.y};
}

template <typename T>
constexpr Vector2<T> operator*(const Vector2<T> &lhs, T rhs) {
    return {lhs.x * rhs, lhs.y * rhs};
}

template <typename T>
constexpr Vector2<T> operator/(const Vector2<T> &lhs, T rhs) {
    return {lhs.x / rhs, lhs.y / rhs};
}

template <typename T>
Vector2<T> &operator+=(Vector2<T> &lhs, const Vector2<T> &rhs) {
    lhs.x += rhs.x;
    lhs.y += rhs.y;
    return lhs;
}

struct Color {
    Uint8 r = 0;
    Uint8 g = 0;
    Uint8 b = 0;
    Uint8 a = 255;

    constexpr Color() = default;
    constexpr Color(Uint8 red, Uint8 green, Uint8 blue, Uint8 alpha = 255)
        : r(red), g(green), b(blue), a(alpha) {}

    static const Color Black;
    static const Color White;
    static const Color Transparent;
};

template <typename T>
struct Rect {
    T left{};
    T top{};
    T width{};
    T height{};

    constexpr Rect() = default;
    constexpr Rect(T rectLeft, T rectTop, T rectWidth, T rectHeight)
        : left(rectLeft), top(rectTop), width(rectWidth), height(rectHeight) {}

    template <typename U>
    explicit constexpr Rect(const Rect<U> &other)
        : left(static_cast<T>(other.left)),
          top(static_cast<T>(other.top)),
          width(static_cast<T>(other.width)),
          height(static_cast<T>(other.height)) {}

    [[nodiscard]] bool contains(const Vector2<T> &point) const {
        return point.x >= left && point.y >= top &&
               point.x <= left + width && point.y <= top + height;
    }
};

using FloatRect = Rect<float>;
using IntRect = Rect<int>;

class Time {
public:
    constexpr Time() = default;
    explicit constexpr Time(float seconds) : m_Seconds(seconds) {}

    [[nodiscard]] constexpr float asSeconds() const { return m_Seconds; }

    Time &operator+=(const Time &other) {
        m_Seconds += other.m_Seconds;
        return *this;
    }

    static const Time Zero;

private:
    float m_Seconds = 0.0f;
};

Time seconds(float value);

class Clock {
public:
    Clock();
    Time restart();

private:
    double m_LastSeconds = 0.0;
};

class Image {
public:
    bool loadFromMemory(const void *data, std::size_t size);
    void create(unsigned int width, unsigned int height, const Color &color = Color::Transparent);

    [[nodiscard]] Vector2u getSize() const;
    [[nodiscard]] Color getPixel(unsigned int x, unsigned int y) const;
    void setPixel(unsigned int x, unsigned int y, const Color &color);

    [[nodiscard]] const std::vector<std::uint8_t> &pixels() const { return m_Pixels; }

private:
    Vector2u m_Size;
    std::vector<std::uint8_t> m_Pixels;
};

class Texture {
public:
    Texture() = default;
    Texture(const Texture &other);
    Texture &operator=(const Texture &other);
    Texture(Texture &&other) noexcept;
    Texture &operator=(Texture &&other) noexcept;
    ~Texture();

    bool loadFromMemory(const void *data, std::size_t size);
    bool loadFromImage(const Image &image);

    [[nodiscard]] Image copyToImage() const;
    [[nodiscard]] Vector2u getSize() const;

    void setSmooth(bool smooth);
    [[nodiscard]] bool isSmooth() const;
    void setRepeated(bool repeated);
    [[nodiscard]] bool isRepeated() const;

private:
    SDL_Texture *native(SDL_Renderer *renderer) const;
    void releaseNative() const;

    Image m_Image;
    bool m_Smooth = false;
    bool m_Repeated = false;
    mutable SDL_Texture *m_Texture = nullptr;
    mutable SDL_Renderer *m_TextureRenderer = nullptr;

    friend class Sprite;
    friend class Text;
};

class View {
public:
    View();
    explicit View(const FloatRect &rect);

    void setCenter(const Vector2f &center);
    void setCenter(float x, float y);
    [[nodiscard]] Vector2f getCenter() const;

    void setSize(const Vector2f &size);
    void setSize(float width, float height);
    [[nodiscard]] Vector2f getSize() const;

    void setViewport(const FloatRect &viewport);
    [[nodiscard]] FloatRect getViewport() const;

    void setRotation(float angle);
    [[nodiscard]] float getRotation() const;

private:
    Vector2f m_Center;
    Vector2f m_Size;
    FloatRect m_Viewport{0.0f, 0.0f, 1.0f, 1.0f};
    float m_Rotation = 0.0f;
};

class RenderTarget;

class Drawable {
public:
    virtual ~Drawable() = default;

private:
    virtual void draw(RenderTarget &target) const = 0;

    friend class RenderTarget;
};

class RenderTarget {
public:
    virtual ~RenderTarget() = default;

    void draw(const Drawable &drawable);
    void setView(const View &view);
    [[nodiscard]] const View &getView() const;
    [[nodiscard]] const View &getDefaultView() const;
    [[nodiscard]] Vector2u getSize() const;

    [[nodiscard]] Vector2f mapPixelToCoords(const Vector2i &point) const;
    [[nodiscard]] Vector2f mapPixelToCoords(const Vector2i &point, const View &view) const;
    [[nodiscard]] Vector2i mapCoordsToPixel(const Vector2f &point, const View &view) const;

protected:
    explicit RenderTarget(const Vector2u &size);

    void setTargetSize(const Vector2u &size);
    [[nodiscard]] SDL_Renderer *renderer() const;
    void setRenderer(SDL_Renderer *renderer);
    [[nodiscard]] FloatRect worldToTargetRect(const FloatRect &worldRect) const;
    [[nodiscard]] FloatRect viewportPixels(const View &view) const;

    View m_View;
    View m_DefaultView;
    Vector2u m_Size;
    SDL_Renderer *m_Renderer = nullptr;

    friend class RectangleShape;
    friend class Sprite;
    friend class Text;
};

class Sprite : public Drawable {
public:
    Sprite() = default;
    explicit Sprite(const Texture &texture);

    void setTexture(const Texture &texture, bool resetRect = false);
    [[nodiscard]] const Texture *getTexture() const;
    void setTextureRect(const IntRect &rect);
    [[nodiscard]] IntRect getTextureRect() const;

    void setPosition(float x, float y);
    void setPosition(const Vector2f &position);
    [[nodiscard]] Vector2f getPosition() const;

    void setOrigin(float x, float y);
    void setOrigin(const Vector2f &origin);
    [[nodiscard]] Vector2f getOrigin() const;

    void setScale(float x, float y);
    void setScale(const Vector2f &scale);
    [[nodiscard]] Vector2f getScale() const;

    void setRotation(float angle);
    [[nodiscard]] float getRotation() const;

    void setColor(const Color &color);
    [[nodiscard]] Color getColor() const;

    [[nodiscard]] FloatRect getLocalBounds() const;
    [[nodiscard]] FloatRect getGlobalBounds() const;

private:
    void draw(RenderTarget &target) const override;

    const Texture *m_Texture = nullptr;
    IntRect m_TextureRect;
    Vector2f m_Position;
    Vector2f m_Origin;
    Vector2f m_Scale{1.0f, 1.0f};
    float m_Rotation = 0.0f;
    Color m_Color = Color::White;
};

class Shape : public Drawable {
public:
    void setFillColor(const Color &color);
    [[nodiscard]] Color getFillColor() const;

protected:
    Color m_FillColor = Color::White;
};

class RectangleShape : public Shape {
public:
    RectangleShape() = default;
    explicit RectangleShape(const Vector2f &size);

    void setSize(const Vector2f &size);
    [[nodiscard]] Vector2f getSize() const;

    void setPosition(float x, float y);
    void setPosition(const Vector2f &position);
    [[nodiscard]] Vector2f getPosition() const;

    [[nodiscard]] FloatRect getLocalBounds() const;
    [[nodiscard]] FloatRect getGlobalBounds() const;

private:
    void draw(RenderTarget &target) const override;

    Vector2f m_Size;
    Vector2f m_Position;
};

class Font {
public:
    Font();
    ~Font();

    Font(const Font &) = delete;
    Font &operator=(const Font &) = delete;
    Font(Font &&) noexcept;
    Font &operator=(Font &&) noexcept;

    bool loadFromMemory(const void *data, std::size_t size);
    [[nodiscard]] bool isLoaded() const;

private:
    struct Impl;

    [[nodiscard]] FloatRect measure(const std::string &text, unsigned int characterSize) const;
    [[nodiscard]] Image rasterize(const std::string &text, unsigned int characterSize) const;

    std::unique_ptr<Impl> m_Impl;

    friend class Text;
};

class Text : public Drawable {
public:
    Text();

    void setFont(const Font &font);
    void setString(const std::string &text);
    void setCharacterSize(unsigned int size);
    void setFillColor(const Color &color);
    [[nodiscard]] Color getFillColor() const;

    void setPosition(float x, float y);
    void setPosition(const Vector2f &position);
    [[nodiscard]] Vector2f getPosition() const;

    [[nodiscard]] FloatRect getLocalBounds() const;
    [[nodiscard]] FloatRect getGlobalBounds() const;

private:
    void draw(RenderTarget &target) const override;
    void refreshTexture() const;

    const Font *m_Font = nullptr;
    std::string m_String;
    unsigned int m_CharacterSize = 30;
    Color m_Color = Color::White;
    Vector2f m_Position;
    mutable Texture m_Texture;
    mutable bool m_TextureDirty = true;
};

class RenderTexture : public RenderTarget {
public:
    RenderTexture();
};

struct VideoMode {
    unsigned int width = 0;
    unsigned int height = 0;

    VideoMode() = default;
    VideoMode(unsigned int modeWidth, unsigned int modeHeight)
        : width(modeWidth), height(modeHeight) {}
};

struct Event {
    enum EventType {
        Closed,
        Resized,
        Other
    };

    EventType type = Other;
    SDL_Event native{};
};

class RenderWindow : public RenderTarget {
public:
    RenderWindow();
    RenderWindow(VideoMode mode, const std::string &title);
    ~RenderWindow() override;

    RenderWindow(const RenderWindow &) = delete;
    RenderWindow &operator=(const RenderWindow &) = delete;

    bool create(VideoMode mode, const std::string &title);
    bool isOpen() const;
    void close();
    bool pollEvent(Event &event);
    void clear(const Color &color = Color::Black);
    void display();
    void setVerticalSyncEnabled(bool enabled);
    [[nodiscard]] SDL_Window *nativeWindow() const;
    [[nodiscard]] SDL_Renderer *nativeRenderer() const;

private:
    SDL_Window *m_Window = nullptr;
    bool m_Open = false;
};

class Keyboard {
public:
    enum Key {
        Unknown = -1,
        A,
        B,
        C,
        D,
        E,
        F,
        G,
        H,
        I,
        J,
        K,
        L,
        M,
        N,
        O,
        P,
        Q,
        R,
        S,
        T,
        U,
        V,
        W,
        X,
        Y,
        Z,
        Num0,
        Num1,
        Num2,
        Num3,
        Num4,
        Num5,
        Num6,
        Num7,
        Num8,
        Num9,
        Left,
        Right,
        Up,
        Down,
        Enter,
        Escape,
        Space,
        Tab,
        Backspace,
        LShift,
        RShift,
        LControl,
        RControl,
        F1,
        F2
    };

    static bool isKeyPressed(Key key);
};

class Mouse {
public:
    enum Button {
        Left
    };

    static bool isButtonPressed(Button button);
    static Vector2i getPosition(const RenderWindow &window);
};
} // namespace Paingine2D
