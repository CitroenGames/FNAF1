#include "Paingine/Paingine2D.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

#include <SDL3/SDL.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include <imstb_truetype.h>

namespace {
    constexpr float FallbackGlyphWidth = 0.55f;

    [[nodiscard]] SDL_FRect ToSDLFRect(const Paingine2D::FloatRect &rect) {
        return SDL_FRect{rect.left, rect.top, rect.width, rect.height};
    }
}

namespace Paingine2D {
struct Font::Impl {
    std::vector<unsigned char> data;
    stbtt_fontinfo info{};
    bool loaded = false;
};

Font::Font()
    : m_Impl(std::make_unique<Impl>()) {
}

Font::~Font() = default;
Font::Font(Font &&) noexcept = default;
Font &Font::operator=(Font &&) noexcept = default;

bool Font::loadFromMemory(const void *data, std::size_t size) {
    if (data == nullptr || size == 0) {
        return false;
    }

    m_Impl->data.assign(static_cast<const unsigned char *>(data), static_cast<const unsigned char *>(data) + size);
    const int offset = stbtt_GetFontOffsetForIndex(m_Impl->data.data(), 0);
    if (offset < 0 || !stbtt_InitFont(&m_Impl->info, m_Impl->data.data(), offset)) {
        m_Impl->data.clear();
        m_Impl->loaded = false;
        return false;
    }

    m_Impl->loaded = true;
    return true;
}

bool Font::isLoaded() const {
    return m_Impl && m_Impl->loaded;
}

FloatRect Font::measure(const std::string &text, unsigned int characterSize) const {
    if (!isLoaded() || text.empty()) {
        return FloatRect(0.0f, 0.0f, static_cast<float>(text.size()) * characterSize * FallbackGlyphWidth,
                         static_cast<float>(characterSize));
    }

    const float scale = stbtt_ScaleForPixelHeight(&m_Impl->info, static_cast<float>(characterSize));
    int ascent = 0;
    int descent = 0;
    int lineGap = 0;
    stbtt_GetFontVMetrics(&m_Impl->info, &ascent, &descent, &lineGap);

    float cursor = 0.0f;
    for (std::size_t i = 0; i < text.size(); ++i) {
        const int codepoint = static_cast<unsigned char>(text[i]);
        int advance = 0;
        int lsb = 0;
        stbtt_GetCodepointHMetrics(&m_Impl->info, codepoint, &advance, &lsb);
        cursor += static_cast<float>(advance) * scale;
        if (i + 1 < text.size()) {
            cursor += static_cast<float>(
                          stbtt_GetCodepointKernAdvance(
                              &m_Impl->info,
                              codepoint,
                              static_cast<unsigned char>(text[i + 1]))) *
                      scale;
        }
    }

    const float height = static_cast<float>(ascent - descent + lineGap) * scale;
    return FloatRect(0.0f, 0.0f, std::max(cursor, 1.0f), std::max(height, static_cast<float>(characterSize)));
}

Image Font::rasterize(const std::string &text, unsigned int characterSize) const {
    const FloatRect metrics = measure(text, characterSize);
    const unsigned int width = std::max(1u, static_cast<unsigned int>(std::ceil(metrics.width + 4.0f)));
    const unsigned int height = std::max(1u, static_cast<unsigned int>(std::ceil(metrics.height + 4.0f)));

    Image image;
    image.create(width, height, Color::Transparent);

    if (!isLoaded() || text.empty()) {
        return image;
    }

    std::vector<std::uint8_t> pixels = image.pixels();
    const float scale = stbtt_ScaleForPixelHeight(&m_Impl->info, static_cast<float>(characterSize));
    int ascent = 0;
    int descent = 0;
    int lineGap = 0;
    stbtt_GetFontVMetrics(&m_Impl->info, &ascent, &descent, &lineGap);

    float cursorX = 2.0f;
    const int baseline = static_cast<int>(2.0f + ascent * scale);

    for (std::size_t i = 0; i < text.size(); ++i) {
        const int codepoint = static_cast<unsigned char>(text[i]);
        int advance = 0;
        int lsb = 0;
        stbtt_GetCodepointHMetrics(&m_Impl->info, codepoint, &advance, &lsb);

        int glyphWidth = 0;
        int glyphHeight = 0;
        int offsetX = 0;
        int offsetY = 0;
        unsigned char *bitmap = stbtt_GetCodepointBitmap(
            &m_Impl->info,
            0.0f,
            scale,
            codepoint,
            &glyphWidth,
            &glyphHeight,
            &offsetX,
            &offsetY);

        const int dstX = static_cast<int>(cursorX) + offsetX;
        const int dstY = baseline + offsetY;
        for (int y = 0; y < glyphHeight; ++y) {
            const int targetY = dstY + y;
            if (targetY < 0 || targetY >= static_cast<int>(height)) {
                continue;
            }

            for (int x = 0; x < glyphWidth; ++x) {
                const int targetX = dstX + x;
                if (targetX < 0 || targetX >= static_cast<int>(width)) {
                    continue;
                }

                const unsigned char alpha = bitmap[y * glyphWidth + x];
                const std::size_t pixelIndex =
                    (static_cast<std::size_t>(targetY) * width + static_cast<std::size_t>(targetX)) * 4;
                pixels[pixelIndex + 0] = 255;
                pixels[pixelIndex + 1] = 255;
                pixels[pixelIndex + 2] = 255;
                pixels[pixelIndex + 3] = std::max(pixels[pixelIndex + 3], alpha);
            }
        }

        stbtt_FreeBitmap(bitmap, nullptr);

        cursorX += static_cast<float>(advance) * scale;
        if (i + 1 < text.size()) {
            cursorX += static_cast<float>(
                           stbtt_GetCodepointKernAdvance(
                               &m_Impl->info,
                               codepoint,
                               static_cast<unsigned char>(text[i + 1]))) *
                       scale;
        }
    }

    for (unsigned int y = 0; y < height; ++y) {
        for (unsigned int x = 0; x < width; ++x) {
            const std::size_t pixelIndex = (static_cast<std::size_t>(y) * width + x) * 4;
            image.setPixel(
                x,
                y,
                Color(
                    pixels[pixelIndex + 0],
                    pixels[pixelIndex + 1],
                    pixels[pixelIndex + 2],
                    pixels[pixelIndex + 3]));
        }
    }

    return image;
}

Text::Text() = default;

void Text::setFont(const Font &font) {
    m_Font = &font;
    m_TextureDirty = true;
}

void Text::setString(const std::string &text) {
    m_String = text;
    m_TextureDirty = true;
}

void Text::setCharacterSize(unsigned int size) {
    m_CharacterSize = size;
    m_TextureDirty = true;
}

void Text::setFillColor(const Color &color) {
    m_Color = color;
}

Color Text::getFillColor() const {
    return m_Color;
}

void Text::setPosition(float x, float y) {
    m_Position = Vector2f(x, y);
}

void Text::setPosition(const Vector2f &position) {
    m_Position = position;
}

Vector2f Text::getPosition() const {
    return m_Position;
}

FloatRect Text::getLocalBounds() const {
    if (m_Font != nullptr) {
        return m_Font->measure(m_String, m_CharacterSize);
    }

    return FloatRect(0.0f, 0.0f, static_cast<float>(m_String.size()) * m_CharacterSize * FallbackGlyphWidth,
                     static_cast<float>(m_CharacterSize));
}

FloatRect Text::getGlobalBounds() const {
    const FloatRect local = getLocalBounds();
    return FloatRect(m_Position.x + local.left, m_Position.y + local.top, local.width, local.height);
}

void Text::draw(RenderTarget &target) const {
    if (target.renderer() == nullptr || m_String.empty() || m_Font == nullptr) {
        return;
    }

    refreshTexture();
    SDL_Texture *texture = m_Texture.native(target.renderer());
    if (texture == nullptr) {
        return;
    }

    SDL_SetTextureColorMod(texture, m_Color.r, m_Color.g, m_Color.b);
    SDL_SetTextureAlphaMod(texture, m_Color.a);
    const Vector2u textureSize = m_Texture.getSize();
    const SDL_FRect source{0.0f, 0.0f, static_cast<float>(textureSize.x), static_cast<float>(textureSize.y)};
    const SDL_FRect destination = ToSDLFRect(target.worldToTargetRect(
        FloatRect(m_Position.x, m_Position.y, source.w, source.h)));
    SDL_RenderTexture(target.renderer(), texture, &source, &destination);
}

void Text::refreshTexture() const {
    if (!m_TextureDirty || m_Font == nullptr) {
        return;
    }

    m_Texture.loadFromImage(m_Font->rasterize(m_String, m_CharacterSize));
    m_TextureDirty = false;
}
} // namespace Paingine2D
