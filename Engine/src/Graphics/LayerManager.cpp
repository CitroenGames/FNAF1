#include "Graphics/LayerManager.h"

#include <map>

#include "Graphics/Layer.h"

namespace {
    std::map<int, Layer> g_Layers;
    std::map<const Paingine2D::Drawable *, int> g_DrawableToLayer;
}

void LayerManager::AddDrawable(int layer, const Paingine2D::Drawable *drawable) {
    if (!drawable) return;

    auto it = g_DrawableToLayer.find(drawable);
    if (it != g_DrawableToLayer.end()) {
        int oldLayer = it->second;
        g_Layers[oldLayer].removeDrawable(drawable);
    }

    g_Layers[layer].addDrawable(drawable);
    g_DrawableToLayer[drawable] = layer;
}

void LayerManager::ChangeLayer(const Paingine2D::Drawable *drawable, int newLayer) {
    if (!drawable) return;

    auto it = g_DrawableToLayer.find(drawable);
    if (it != g_DrawableToLayer.end()) {
        int currentLayer = it->second;
        if (currentLayer != newLayer) {
            g_Layers[currentLayer].removeDrawable(drawable);
            g_Layers[newLayer].addDrawable(drawable);
            it->second = newLayer;
        }
    } else {
        AddDrawable(newLayer, drawable);
    }
}

void LayerManager::RemoveDrawable(const Paingine2D::Drawable *drawable) {
    if (!drawable) return;

    auto it = g_DrawableToLayer.find(drawable);
    if (it != g_DrawableToLayer.end()) {
        int layer = it->second;
        g_Layers[layer].removeDrawable(drawable);
        g_DrawableToLayer.erase(it);
    }
}

void LayerManager::Clear() {
    for (auto &layer: g_Layers) {
        layer.second.clear();
    }

    g_Layers.clear();
    g_DrawableToLayer.clear();
}

void LayerManager::Draw(Paingine2D::RenderWindow &window) {
    for (const auto &layer: g_Layers) {
        layer.second.draw(window);
    }
}
