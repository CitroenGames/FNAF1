#include "imgui-Paingine.h"

#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_sdlrenderer3.h"
#include "imgui.h"

namespace {
    bool g_Initialized = false;
    bool g_OwnsContext = false;
}

namespace ImGui::Paingine {
bool Init(Paingine2D::RenderWindow &window, bool loadDefaultFont) {
    if (window.nativeWindow() == nullptr || window.nativeRenderer() == nullptr) {
        return false;
    }

    if (ImGui::GetCurrentContext() == nullptr) {
        ImGui::CreateContext();
        g_OwnsContext = true;
    }

    ImGuiIO &io = ImGui::GetIO();
    if (loadDefaultFont && io.Fonts->Fonts.empty()) {
        io.Fonts->AddFontDefault();
    }

    if (!ImGui_ImplSDL3_InitForSDLRenderer(window.nativeWindow(), window.nativeRenderer())) {
        return false;
    }

    if (!ImGui_ImplSDLRenderer3_Init(window.nativeRenderer())) {
        ImGui_ImplSDL3_Shutdown();
        return false;
    }

    g_Initialized = true;
    return true;
}

void ProcessEvent(const Paingine2D::Event &event) {
    if (g_Initialized) {
        ImGui_ImplSDL3_ProcessEvent(&event.native);
    }
}

void Update(Paingine2D::RenderWindow &window, Paingine2D::Time dt) {
    (void) window;
    (void) dt;
    if (!g_Initialized) {
        return;
    }

    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
}

void Render(Paingine2D::RenderWindow &window) {
    if (!g_Initialized || window.nativeRenderer() == nullptr) {
        return;
    }

    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), window.nativeRenderer());
}

void Shutdown() {
    if (g_Initialized) {
        ImGui_ImplSDLRenderer3_Shutdown();
        ImGui_ImplSDL3_Shutdown();
    }

    if (g_OwnsContext && ImGui::GetCurrentContext() != nullptr) {
        ImGui::DestroyContext();
    }

    g_Initialized = false;
    g_OwnsContext = false;
}
} // namespace ImGui::Paingine
