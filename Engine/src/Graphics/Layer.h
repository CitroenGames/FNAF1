#pragma once

#include <vector>

#include <Paingine/Graphics/Drawable.hpp>
#include <Paingine/Graphics/RenderWindow.hpp>

class Layer {
public:
    Layer() = default;

    ~Layer() = default;

    void addDrawable(const Paingine2D::Drawable *drawable);

    void removeDrawable(const Paingine2D::Drawable *drawable);

    void draw(Paingine2D::RenderWindow &window) const;

    void clear();

private:
    std::vector<const Paingine2D::Drawable *> m_Drawables;
};
