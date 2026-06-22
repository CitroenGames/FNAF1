#include "Input/InputActionMap.h"

#include <algorithm>
#include <utility>

namespace {
    InputActionMap g_Actions;

    bool IsAnyKeyDown(const std::vector<Paingine2D::Keyboard::Key> &keys) {
        return std::any_of(keys.begin(), keys.end(), [](Paingine2D::Keyboard::Key key) {
            return Paingine2D::Keyboard::isKeyPressed(key);
        });
    }
}

void InputActionMap::BindAction(const std::string &action, Paingine2D::Keyboard::Key key) {
    BindAction(action, std::vector<Paingine2D::Keyboard::Key>{key});
}

void InputActionMap::BindAction(const std::string &action, std::vector<Paingine2D::Keyboard::Key> keys) {
    m_Bindings[action].keys = std::move(keys);
}

void InputActionMap::UnbindAction(const std::string &action) {
    m_Bindings.erase(action);
}

void InputActionMap::Clear() {
    m_Bindings.clear();
}

void InputActionMap::Update() {
    for (auto &[action, binding]: m_Bindings) {
        (void) action;
        const bool wasDown = binding.down;
        binding.down = IsAnyKeyDown(binding.keys);
        binding.pressed = binding.down && !wasDown;
        binding.released = !binding.down && wasDown;
    }
}

bool InputActionMap::IsDown(const std::string &action) const {
    if (const Binding *binding = GetBinding(action)) {
        return binding->down;
    }

    return false;
}

bool InputActionMap::WasPressed(const std::string &action) const {
    if (const Binding *binding = GetBinding(action)) {
        return binding->pressed;
    }

    return false;
}

bool InputActionMap::WasReleased(const std::string &action) const {
    if (const Binding *binding = GetBinding(action)) {
        return binding->released;
    }

    return false;
}

const InputActionMap::Binding *InputActionMap::GetBinding(const std::string &action) const {
    const auto it = m_Bindings.find(action);
    if (it == m_Bindings.end()) {
        return nullptr;
    }

    return &it->second;
}

InputActionMap &Input::Actions() {
    return g_Actions;
}

void Input::ConfigureNarrativeDefaults() {
    g_Actions.Clear();
    g_Actions.BindAction("move_left", {Paingine2D::Keyboard::Left, Paingine2D::Keyboard::A});
    g_Actions.BindAction("move_right", {Paingine2D::Keyboard::Right, Paingine2D::Keyboard::D});
    g_Actions.BindAction("move_up", {Paingine2D::Keyboard::Up, Paingine2D::Keyboard::W});
    g_Actions.BindAction("move_down", {Paingine2D::Keyboard::Down, Paingine2D::Keyboard::S});
    g_Actions.BindAction("confirm", {Paingine2D::Keyboard::Enter, Paingine2D::Keyboard::Space, Paingine2D::Keyboard::Z});
    g_Actions.BindAction("cancel", {Paingine2D::Keyboard::Escape, Paingine2D::Keyboard::B});
    g_Actions.BindAction("run", {Paingine2D::Keyboard::LShift, Paingine2D::Keyboard::RShift});
    g_Actions.BindAction("inspect", {Paingine2D::Keyboard::X});
    g_Actions.BindAction("memory", {Paingine2D::Keyboard::Y});
    g_Actions.BindAction("dialogue_prev", {Paingine2D::Keyboard::Q});
    g_Actions.BindAction("dialogue_next", {Paingine2D::Keyboard::E});
    g_Actions.BindAction("journal", {Paingine2D::Keyboard::J, Paingine2D::Keyboard::Tab});
}

void Input::Update() {
    g_Actions.Update();
}

bool Input::IsActionDown(const std::string &action) {
    return g_Actions.IsDown(action);
}

bool Input::WasActionPressed(const std::string &action) {
    return g_Actions.WasPressed(action);
}

bool Input::WasActionReleased(const std::string &action) {
    return g_Actions.WasReleased(action);
}
