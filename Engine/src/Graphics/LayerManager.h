#pragma once

#include <Paingine/Graphics/Drawable.hpp>
#include <Paingine/Graphics/RenderWindow.hpp>

class LayerManager {
public:
    static void AddDrawable(int layer, const Paingine2D::Drawable *drawable);

    static void ChangeLayer(const Paingine2D::Drawable *drawable, int newLayer);

    static void RemoveDrawable(const Paingine2D::Drawable *drawable);

    static void Clear();

    static void Draw(Paingine2D::RenderWindow &window);
};
