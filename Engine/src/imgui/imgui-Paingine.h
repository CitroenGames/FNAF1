#pragma once

#include <Paingine/Graphics/RenderWindow.hpp>
#include <Paingine/System/Time.hpp>
#include <Paingine/Window/Event.hpp>

#include "imgui-Paingine_export.h"

namespace ImGui::Paingine {
IMGUI_PAINGINE_API bool Init(Paingine2D::RenderWindow &window, bool loadDefaultFont = true);
IMGUI_PAINGINE_API void ProcessEvent(const Paingine2D::Event &event);
IMGUI_PAINGINE_API void Update(Paingine2D::RenderWindow &window, Paingine2D::Time dt);
IMGUI_PAINGINE_API void Render(Paingine2D::RenderWindow &window);
IMGUI_PAINGINE_API void Shutdown();
} // namespace ImGui::Paingine
