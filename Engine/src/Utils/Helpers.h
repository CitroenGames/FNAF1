#pragma once

#include <memory>

#include <Paingine/Graphics/Texture.hpp>

std::shared_ptr<Paingine2D::Texture> MakeTextureTransparent(const std::shared_ptr<Paingine2D::Texture> &inputTexture,
                                                    float alpha = 0.5f);

std::shared_ptr<Paingine2D::Texture> ProcessText(const std::shared_ptr<Paingine2D::Texture> &inputTexture,
                                         int tolerance = 5);

std::shared_ptr<Paingine2D::Texture> RemoveBlackBackground(const std::shared_ptr<Paingine2D::Texture> &inputTexture,
                                                   int threshold = 30);
