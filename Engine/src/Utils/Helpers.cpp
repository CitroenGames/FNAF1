#include "Utils/Helpers.h"

#include <cstdint>
#include <cstdlib>

#include <Paingine/Graphics/Color.hpp>
#include <Paingine/Graphics/Image.hpp>

std::shared_ptr<Paingine2D::Texture> MakeTextureTransparent(const std::shared_ptr<Paingine2D::Texture> &inputTexture,
                                                    float alpha) {
    if (!inputTexture || !inputTexture->getSize().x || !inputTexture->getSize().y) {
        return nullptr;
    }

    auto outputTexture = std::make_shared<Paingine2D::Texture>();
    outputTexture->setSmooth(inputTexture->isSmooth());
    outputTexture->setRepeated(inputTexture->isRepeated());

    Paingine2D::Image image = inputTexture->copyToImage();
    const auto size = image.getSize();

    uint8_t alphaValue = static_cast<uint8_t>(alpha * 255);

    for (unsigned int x = 0; x < size.x; x++) {
        for (unsigned int y = 0; y < size.y; y++) {
            const Paingine2D::Color pixel = image.getPixel(x, y);
            image.setPixel(x, y, Paingine2D::Color(pixel.r, pixel.g, pixel.b, alphaValue));
        }
    }

    if (!outputTexture->loadFromImage(image)) {
        return nullptr;
    }

    return outputTexture;
}

std::shared_ptr<Paingine2D::Texture> ProcessText(const std::shared_ptr<Paingine2D::Texture> &inputTexture,
                                         int tolerance) {
    if (!inputTexture || !inputTexture->getSize().x || !inputTexture->getSize().y) {
        return nullptr;
    }

    auto outputTexture = std::make_shared<Paingine2D::Texture>();
    outputTexture->setSmooth(inputTexture->isSmooth());
    outputTexture->setRepeated(inputTexture->isRepeated());

    Paingine2D::Image image = inputTexture->copyToImage();
    const auto size = image.getSize();

    for (unsigned int x = 0; x < size.x; x++) {
        for (unsigned int y = 0; y < size.y; y++) {
            const Paingine2D::Color pixel = image.getPixel(x, y);

            bool isGrey = std::abs(pixel.r - pixel.g) <= tolerance &&
                          std::abs(pixel.g - pixel.b) <= tolerance &&
                          std::abs(pixel.r - pixel.b) <= tolerance &&
                          pixel.r < 240;

            if (isGrey) {
                image.setPixel(x, y, Paingine2D::Color(pixel.r, pixel.g, pixel.b, 0));
            }
        }
    }

    if (!outputTexture->loadFromImage(image)) {
        return nullptr;
    }

    return outputTexture;
}

std::shared_ptr<Paingine2D::Texture> RemoveBlackBackground(const std::shared_ptr<Paingine2D::Texture> &inputTexture,
                                                   int threshold) {
    if (!inputTexture || !inputTexture->getSize().x || !inputTexture->getSize().y) {
        return nullptr;
    }

    auto outputTexture = std::make_shared<Paingine2D::Texture>();
    outputTexture->setSmooth(inputTexture->isSmooth());
    outputTexture->setRepeated(inputTexture->isRepeated());

    Paingine2D::Image image = inputTexture->copyToImage();
    const auto size = image.getSize();

    for (unsigned int x = 0; x < size.x; x++) {
        for (unsigned int y = 0; y < size.y; y++) {
            const Paingine2D::Color pixel = image.getPixel(x, y);

            if (pixel.r <= threshold && pixel.g <= threshold && pixel.b <= threshold) {
                image.setPixel(x, y, Paingine2D::Color(pixel.r, pixel.g, pixel.b, 0));
            }
        }
    }

    if (!outputTexture->loadFromImage(image)) {
        return nullptr;
    }

    return outputTexture;
}
