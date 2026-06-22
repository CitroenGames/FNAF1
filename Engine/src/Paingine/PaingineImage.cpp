#include "Paingine/Paingine2D.h"

#include <cstddef>
#include <iostream>
#include <utility>

#include <SDL3/SDL.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Paingine2D {
bool Image::loadFromMemory(const void *data, std::size_t size) {
    int width = 0;
    int height = 0;
    int channels = 0;
    unsigned char *pixels = stbi_load_from_memory(
        static_cast<const stbi_uc *>(data),
        static_cast<int>(size),
        &width,
        &height,
        &channels,
        4);

    if (pixels == nullptr || width <= 0 || height <= 0) {
        stbi_image_free(pixels);
        return false;
    }

    m_Size = Vector2u(static_cast<unsigned int>(width), static_cast<unsigned int>(height));
    const std::size_t byteCount = static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * 4;
    m_Pixels.assign(pixels, pixels + byteCount);
    stbi_image_free(pixels);
    return true;
}

void Image::create(unsigned int width, unsigned int height, const Color &color) {
    m_Size = Vector2u(width, height);
    m_Pixels.resize(static_cast<std::size_t>(width) * height * 4);

    for (std::size_t i = 0; i < m_Pixels.size(); i += 4) {
        m_Pixels[i + 0] = color.r;
        m_Pixels[i + 1] = color.g;
        m_Pixels[i + 2] = color.b;
        m_Pixels[i + 3] = color.a;
    }
}

Vector2u Image::getSize() const {
    return m_Size;
}

Color Image::getPixel(unsigned int x, unsigned int y) const {
    if (x >= m_Size.x || y >= m_Size.y || m_Pixels.empty()) {
        return Color::Transparent;
    }

    const std::size_t index = (static_cast<std::size_t>(y) * m_Size.x + x) * 4;
    return Color(m_Pixels[index + 0], m_Pixels[index + 1], m_Pixels[index + 2], m_Pixels[index + 3]);
}

void Image::setPixel(unsigned int x, unsigned int y, const Color &color) {
    if (x >= m_Size.x || y >= m_Size.y || m_Pixels.empty()) {
        return;
    }

    const std::size_t index = (static_cast<std::size_t>(y) * m_Size.x + x) * 4;
    m_Pixels[index + 0] = color.r;
    m_Pixels[index + 1] = color.g;
    m_Pixels[index + 2] = color.b;
    m_Pixels[index + 3] = color.a;
}

Texture::Texture(const Texture &other)
    : m_Image(other.m_Image),
      m_Smooth(other.m_Smooth),
      m_Repeated(other.m_Repeated) {
}

Texture &Texture::operator=(const Texture &other) {
    if (this != &other) {
        releaseNative();
        m_Image = other.m_Image;
        m_Smooth = other.m_Smooth;
        m_Repeated = other.m_Repeated;
    }
    return *this;
}

Texture::Texture(Texture &&other) noexcept
    : m_Image(std::move(other.m_Image)),
      m_Smooth(other.m_Smooth),
      m_Repeated(other.m_Repeated),
      m_Texture(other.m_Texture),
      m_TextureRenderer(other.m_TextureRenderer) {
    other.m_Texture = nullptr;
    other.m_TextureRenderer = nullptr;
}

Texture &Texture::operator=(Texture &&other) noexcept {
    if (this != &other) {
        releaseNative();
        m_Image = std::move(other.m_Image);
        m_Smooth = other.m_Smooth;
        m_Repeated = other.m_Repeated;
        m_Texture = other.m_Texture;
        m_TextureRenderer = other.m_TextureRenderer;
        other.m_Texture = nullptr;
        other.m_TextureRenderer = nullptr;
    }
    return *this;
}

Texture::~Texture() {
    releaseNative();
}

bool Texture::loadFromMemory(const void *data, std::size_t size) {
    Image image;
    if (!image.loadFromMemory(data, size)) {
        return false;
    }

    return loadFromImage(image);
}

bool Texture::loadFromImage(const Image &image) {
    if (image.getSize().x == 0 || image.getSize().y == 0 || image.pixels().empty()) {
        return false;
    }

    releaseNative();
    m_Image = image;
    return true;
}

Image Texture::copyToImage() const {
    return m_Image;
}

Vector2u Texture::getSize() const {
    return m_Image.getSize();
}

void Texture::setSmooth(bool smooth) {
    m_Smooth = smooth;
}

bool Texture::isSmooth() const {
    return m_Smooth;
}

void Texture::setRepeated(bool repeated) {
    m_Repeated = repeated;
}

bool Texture::isRepeated() const {
    return m_Repeated;
}

SDL_Texture *Texture::native(SDL_Renderer *renderer) const {
    if (renderer == nullptr || m_Image.getSize().x == 0 || m_Image.getSize().y == 0 || m_Image.pixels().empty()) {
        return nullptr;
    }

    if (m_Texture != nullptr && m_TextureRenderer == renderer) {
        return m_Texture;
    }

    releaseNative();
    m_Texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA32,
        SDL_TEXTUREACCESS_STATIC,
        static_cast<int>(m_Image.getSize().x),
        static_cast<int>(m_Image.getSize().y));

    if (m_Texture == nullptr) {
        std::cerr << "SDL texture creation failed: " << SDL_GetError() << std::endl;
        return nullptr;
    }

    SDL_SetTextureBlendMode(m_Texture, SDL_BLENDMODE_BLEND);

    const SDL_Rect rect{0, 0, static_cast<int>(m_Image.getSize().x), static_cast<int>(m_Image.getSize().y)};
    if (!SDL_UpdateTexture(m_Texture, &rect, m_Image.pixels().data(), static_cast<int>(m_Image.getSize().x * 4))) {
        std::cerr << "SDL texture upload failed: " << SDL_GetError() << std::endl;
        releaseNative();
        return nullptr;
    }

    m_TextureRenderer = renderer;
    return m_Texture;
}

void Texture::releaseNative() const {
    if (m_Texture != nullptr) {
        SDL_DestroyTexture(m_Texture);
        m_Texture = nullptr;
        m_TextureRenderer = nullptr;
    }
}
} // namespace Paingine2D
