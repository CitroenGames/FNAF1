#include "Utils/Helpers.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>

#include <Paingine/Graphics/Color.hpp>
#include <Paingine/Graphics/Image.hpp>

namespace {
    // The three public helpers differ only in the per-pixel rule they apply, so the
    // copy-out / iterate / load-back scaffolding lives here once.
    template<typename PixelOp>
    std::shared_ptr<Paingine2D::Texture> TransformPixels(
        const std::shared_ptr<Paingine2D::Texture> &inputTexture,
        PixelOp transform) {
        if (!inputTexture || !inputTexture->getSize().x || !inputTexture->getSize().y) {
            return nullptr;
        }

        Paingine2D::Image image = inputTexture->copyToImage();
        const auto size = image.getSize();

        // Row-major order: y outer, x inner, so the scan follows memory layout.
        for (unsigned int y = 0; y < size.y; ++y) {
            for (unsigned int x = 0; x < size.x; ++x) {
                image.setPixel(x, y, transform(image.getPixel(x, y)));
            }
        }

        auto outputTexture = std::make_shared<Paingine2D::Texture>();
        outputTexture->setSmooth(inputTexture->isSmooth());
        outputTexture->setRepeated(inputTexture->isRepeated());
        if (!outputTexture->loadFromImage(image)) {
            return nullptr;
        }

        return outputTexture;
    }

    Paingine2D::Color WithAlpha(const Paingine2D::Color &pixel, uint8_t alpha) {
        return Paingine2D::Color(pixel.r, pixel.g, pixel.b, alpha);
    }
}

std::shared_ptr<Paingine2D::Texture> MakeTextureTransparent(const std::shared_ptr<Paingine2D::Texture> &inputTexture,
                                                            float alpha) {
    const auto alphaValue = static_cast<uint8_t>(std::clamp(alpha, 0.0f, 1.0f) * 255.0f);
    return TransformPixels(inputTexture, [alphaValue](const Paingine2D::Color &pixel) {
        return WithAlpha(pixel, alphaValue);
    });
}

std::shared_ptr<Paingine2D::Texture> ProcessText(const std::shared_ptr<Paingine2D::Texture> &inputTexture,
                                                 int tolerance) {
    return TransformPixels(inputTexture, [tolerance](const Paingine2D::Color &pixel) {
        const bool isGrey = std::abs(pixel.r - pixel.g) <= tolerance &&
                            std::abs(pixel.g - pixel.b) <= tolerance &&
                            std::abs(pixel.r - pixel.b) <= tolerance &&
                            pixel.r < 240;

        return isGrey ? WithAlpha(pixel, 0) : pixel;
    });
}

std::shared_ptr<Paingine2D::Texture> RemoveBlackBackground(const std::shared_ptr<Paingine2D::Texture> &inputTexture,
                                                           int threshold) {
    return TransformPixels(inputTexture, [threshold](const Paingine2D::Color &pixel) {
        const bool isBackground = pixel.r <= threshold && pixel.g <= threshold && pixel.b <= threshold;
        return isBackground ? WithAlpha(pixel, 0) : pixel;
    });
}
