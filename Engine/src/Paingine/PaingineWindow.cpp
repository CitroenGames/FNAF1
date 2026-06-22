#include "Paingine/Paingine2D.h"

#include <algorithm>
#include <cmath>
#include <iostream>

#include <SDL3/SDL.h>

namespace {
    [[nodiscard]] SDL_Scancode ToScancode(Paingine2D::Keyboard::Key key) {
        switch (key) {
            case Paingine2D::Keyboard::A:
                return SDL_SCANCODE_A;
            case Paingine2D::Keyboard::B:
                return SDL_SCANCODE_B;
            case Paingine2D::Keyboard::C:
                return SDL_SCANCODE_C;
            case Paingine2D::Keyboard::D:
                return SDL_SCANCODE_D;
            case Paingine2D::Keyboard::E:
                return SDL_SCANCODE_E;
            case Paingine2D::Keyboard::F:
                return SDL_SCANCODE_F;
            case Paingine2D::Keyboard::G:
                return SDL_SCANCODE_G;
            case Paingine2D::Keyboard::H:
                return SDL_SCANCODE_H;
            case Paingine2D::Keyboard::I:
                return SDL_SCANCODE_I;
            case Paingine2D::Keyboard::J:
                return SDL_SCANCODE_J;
            case Paingine2D::Keyboard::K:
                return SDL_SCANCODE_K;
            case Paingine2D::Keyboard::L:
                return SDL_SCANCODE_L;
            case Paingine2D::Keyboard::M:
                return SDL_SCANCODE_M;
            case Paingine2D::Keyboard::N:
                return SDL_SCANCODE_N;
            case Paingine2D::Keyboard::O:
                return SDL_SCANCODE_O;
            case Paingine2D::Keyboard::P:
                return SDL_SCANCODE_P;
            case Paingine2D::Keyboard::Q:
                return SDL_SCANCODE_Q;
            case Paingine2D::Keyboard::R:
                return SDL_SCANCODE_R;
            case Paingine2D::Keyboard::S:
                return SDL_SCANCODE_S;
            case Paingine2D::Keyboard::T:
                return SDL_SCANCODE_T;
            case Paingine2D::Keyboard::U:
                return SDL_SCANCODE_U;
            case Paingine2D::Keyboard::V:
                return SDL_SCANCODE_V;
            case Paingine2D::Keyboard::W:
                return SDL_SCANCODE_W;
            case Paingine2D::Keyboard::X:
                return SDL_SCANCODE_X;
            case Paingine2D::Keyboard::Y:
                return SDL_SCANCODE_Y;
            case Paingine2D::Keyboard::Z:
                return SDL_SCANCODE_Z;
            case Paingine2D::Keyboard::Num0:
                return SDL_SCANCODE_0;
            case Paingine2D::Keyboard::Num1:
                return SDL_SCANCODE_1;
            case Paingine2D::Keyboard::Num2:
                return SDL_SCANCODE_2;
            case Paingine2D::Keyboard::Num3:
                return SDL_SCANCODE_3;
            case Paingine2D::Keyboard::Num4:
                return SDL_SCANCODE_4;
            case Paingine2D::Keyboard::Num5:
                return SDL_SCANCODE_5;
            case Paingine2D::Keyboard::Num6:
                return SDL_SCANCODE_6;
            case Paingine2D::Keyboard::Num7:
                return SDL_SCANCODE_7;
            case Paingine2D::Keyboard::Num8:
                return SDL_SCANCODE_8;
            case Paingine2D::Keyboard::Num9:
                return SDL_SCANCODE_9;
            case Paingine2D::Keyboard::Left:
                return SDL_SCANCODE_LEFT;
            case Paingine2D::Keyboard::Right:
                return SDL_SCANCODE_RIGHT;
            case Paingine2D::Keyboard::Up:
                return SDL_SCANCODE_UP;
            case Paingine2D::Keyboard::Down:
                return SDL_SCANCODE_DOWN;
            case Paingine2D::Keyboard::Enter:
                return SDL_SCANCODE_RETURN;
            case Paingine2D::Keyboard::Escape:
                return SDL_SCANCODE_ESCAPE;
            case Paingine2D::Keyboard::Space:
                return SDL_SCANCODE_SPACE;
            case Paingine2D::Keyboard::Tab:
                return SDL_SCANCODE_TAB;
            case Paingine2D::Keyboard::Backspace:
                return SDL_SCANCODE_BACKSPACE;
            case Paingine2D::Keyboard::LShift:
                return SDL_SCANCODE_LSHIFT;
            case Paingine2D::Keyboard::RShift:
                return SDL_SCANCODE_RSHIFT;
            case Paingine2D::Keyboard::LControl:
                return SDL_SCANCODE_LCTRL;
            case Paingine2D::Keyboard::RControl:
                return SDL_SCANCODE_RCTRL;
            case Paingine2D::Keyboard::F1:
                return SDL_SCANCODE_F1;
            case Paingine2D::Keyboard::F2:
                return SDL_SCANCODE_F2;
            default:
                return SDL_SCANCODE_UNKNOWN;
        }
    }
}

namespace Paingine2D {
RenderWindow::RenderWindow()
    : RenderTarget(Vector2u(0, 0)) {
}

RenderWindow::RenderWindow(VideoMode mode, const std::string &title)
    : RenderWindow() {
    create(mode, title);
}

RenderWindow::~RenderWindow() {
    close();
    if (renderer() != nullptr) {
        SDL_DestroyRenderer(renderer());
        setRenderer(nullptr);
    }

    if (m_Window != nullptr) {
        SDL_DestroyWindow(m_Window);
        m_Window = nullptr;
    }
}

bool RenderWindow::create(VideoMode mode, const std::string &title) {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return false;
    }

    m_Window = SDL_CreateWindow(title.c_str(), static_cast<int>(mode.width), static_cast<int>(mode.height), SDL_WINDOW_RESIZABLE);
    if (m_Window == nullptr) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
        return false;
    }

    SDL_Renderer *createdRenderer = SDL_CreateRenderer(m_Window, nullptr);
    if (createdRenderer == nullptr) {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(m_Window);
        m_Window = nullptr;
        return false;
    }

    setRenderer(createdRenderer);
    setTargetSize(Vector2u(mode.width, mode.height));
    setView(getDefaultView());
    SDL_SetRenderDrawBlendMode(createdRenderer, SDL_BLENDMODE_BLEND);
    m_Open = true;
    return true;
}

bool RenderWindow::isOpen() const {
    return m_Open;
}

void RenderWindow::close() {
    m_Open = false;
}

bool RenderWindow::pollEvent(Event &event) {
    SDL_Event sdlEvent;
    while (SDL_PollEvent(&sdlEvent)) {
        event.native = sdlEvent;
        switch (sdlEvent.type) {
            case SDL_EVENT_QUIT:
            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                event.type = Event::Closed;
                return true;
            case SDL_EVENT_WINDOW_RESIZED: {
                int width = 0;
                int height = 0;
                SDL_GetWindowSize(m_Window, &width, &height);
                setTargetSize(Vector2u(static_cast<unsigned int>(std::max(width, 1)),
                                       static_cast<unsigned int>(std::max(height, 1))));
                event.type = Event::Resized;
                return true;
            }
            default:
                event.type = Event::Other;
                return true;
        }
    }

    return false;
}

void RenderWindow::clear(const Color &color) {
    if (renderer() == nullptr) {
        return;
    }

    SDL_SetRenderDrawColor(renderer(), color.r, color.g, color.b, color.a);
    SDL_RenderClear(renderer());
}

void RenderWindow::display() {
    if (renderer() != nullptr) {
        SDL_RenderPresent(renderer());
    }
}

void RenderWindow::setVerticalSyncEnabled(bool enabled) {
    if (renderer() != nullptr) {
        SDL_SetRenderVSync(renderer(), enabled ? 1 : 0);
    }
}

SDL_Window *RenderWindow::nativeWindow() const {
    return m_Window;
}

SDL_Renderer *RenderWindow::nativeRenderer() const {
    return renderer();
}

bool Keyboard::isKeyPressed(Key key) {
    int keyCount = 0;
    const bool *state = SDL_GetKeyboardState(&keyCount);
    const SDL_Scancode scancode = ToScancode(key);
    return scancode != SDL_SCANCODE_UNKNOWN && scancode < keyCount && state[scancode];
}

bool Mouse::isButtonPressed(Button button) {
    if (button != Left) {
        return false;
    }

    return (SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON_LMASK) != 0;
}

Vector2i Mouse::getPosition(const RenderWindow &window) {
    (void) window;
    float x = 0.0f;
    float y = 0.0f;
    SDL_GetMouseState(&x, &y);
    return Vector2i(static_cast<int>(std::round(x)), static_cast<int>(std::round(y)));
}
} // namespace Paingine2D
