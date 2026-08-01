#include "SceneManager.h"

#include "Audio/AudioManager.h"
#include "Core/Window.h"

namespace {
    std::shared_ptr<Scene> g_ActiveScene;
    std::shared_ptr<Scene> g_QueuedScene;

    // Only a scene that has had Init() called owns any global state (layers, audio),
    // so only such a scene may be torn down with Destroy(). A scene that was queued
    // and then replaced before ever becoming active is simply dropped -- calling
    // Destroy() on it would clear the LayerManager out from under the running scene.
    void DiscardQueuedScene() {
        g_QueuedScene.reset();
    }
}

void SceneManager::Update(double deltaTime) {
    if (g_QueuedScene) {
        auto scene = std::move(g_QueuedScene);
        g_QueuedScene.reset();
        SwitchSceneNow(std::move(scene));
    }

    if (g_ActiveScene) {
        g_ActiveScene->Update(deltaTime);
    }
}

void SceneManager::FixedUpdate() {
    if (g_ActiveScene) {
        g_ActiveScene->FixedUpdate();
    }
}

void SceneManager::Render() {
    if (g_ActiveScene) {
        g_ActiveScene->Render();
    }
}

void SceneManager::QueueSwitchScene(std::shared_ptr<Scene> scene) {
    if (g_QueuedScene && g_QueuedScene != scene) {
        DiscardQueuedScene();
    }

    g_QueuedScene = std::move(scene);
}

void SceneManager::SwitchSceneNow(std::shared_ptr<Scene> queuedScene) {
    if (g_ActiveScene == queuedScene) return;

    AudioManager::GetInstance().StopAllAudio();

    if (g_ActiveScene) {
        g_ActiveScene->Destroy();
        g_ActiveScene.reset();
    }

    g_ActiveScene = std::move(queuedScene);
    Window::UpdateViewport();

    if (g_ActiveScene) {
        g_ActiveScene->Init();
    }
}

std::shared_ptr<Scene> SceneManager::GetActiveScene() {
    return g_ActiveScene;
}

void SceneManager::Destroy() {
    DiscardQueuedScene();

    if (g_ActiveScene) {
        g_ActiveScene->Destroy();
        g_ActiveScene.reset();
    }
}
