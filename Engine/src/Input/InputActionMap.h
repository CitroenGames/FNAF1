#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include <Paingine/Window/Keyboard.hpp>

class InputActionMap {
public:
    struct Binding {
        std::vector<Paingine2D::Keyboard::Key> keys;
        bool down = false;
        bool pressed = false;
        bool released = false;
    };

    void BindAction(const std::string &action, Paingine2D::Keyboard::Key key);
    void BindAction(const std::string &action, std::vector<Paingine2D::Keyboard::Key> keys);
    void UnbindAction(const std::string &action);
    void Clear();

    void Update();

    bool IsDown(const std::string &action) const;
    bool WasPressed(const std::string &action) const;
    bool WasReleased(const std::string &action) const;

    const Binding *GetBinding(const std::string &action) const;

private:
    std::unordered_map<std::string, Binding> m_Bindings;
};

class Input {
public:
    static InputActionMap &Actions();
    static void ConfigureNarrativeDefaults();
    static void Update();

    static bool IsActionDown(const std::string &action);
    static bool WasActionPressed(const std::string &action);
    static bool WasActionReleased(const std::string &action);
};
