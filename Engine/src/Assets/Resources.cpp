#include "Resources.h"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <Paingine/Graphics/Font.hpp>
#include <Paingine/Graphics/Texture.hpp>

#include "Assets/AssetFileSystem.h"
#include "Audio/AudioClip.h"
#include "Pak.h"

namespace {
    std::string g_PakFile;
    std::string g_AssetRoot = "Assets";
    bool g_FileSystemFallbackEnabled = true;
    Pakker *g_PakHandler = nullptr;

    std::map<std::string, std::shared_ptr<Paingine2D::Texture> > g_Textures;
    std::map<std::string, std::shared_ptr<AudioBuffer> > g_AudioBuffers;
    std::map<std::string, std::shared_ptr<Paingine2D::Font> > g_Fonts;
    std::map<std::string, std::shared_ptr<std::vector<uint8_t> > > g_FontBuffers;

    std::shared_ptr<std::vector<uint8_t> > LoadPakAsset(const std::string &filename) {
        if (g_PakHandler == nullptr || g_PakFile.empty()) {
            return nullptr;
        }

        return g_PakHandler->LoadFile(g_PakFile, filename);
    }
}

void Resources::BindPakFile(const std::string &pakFilename) {
    g_PakFile = pakFilename;
}

void Resources::SetAssetRoot(const std::string &rootDirectory) {
    g_AssetRoot = rootDirectory;
}

const std::string &Resources::GetAssetRoot() {
    return g_AssetRoot;
}

void Resources::SetFileSystemFallbackEnabled(bool enabled) {
    g_FileSystemFallbackEnabled = enabled;
}

void Resources::Unload() {
    g_Textures.clear();
    g_AudioBuffers.clear();
    g_Fonts.clear();
    g_FontBuffers.clear();
}

std::shared_ptr<std::vector<uint8_t> > Resources::LoadBytes(const std::string &filename) {
    if (auto data = LoadPakAsset(filename)) {
        return data;
    }

    if (auto data = AssetFileSystem::LoadBytes(g_AssetRoot, filename, g_FileSystemFallbackEnabled)) {
        return data;
    }

    return nullptr;
}

std::shared_ptr<Paingine2D::Texture> Resources::GetTexture(const std::string &filename) {
    auto textureIt = g_Textures.find(filename);
    if (textureIt == g_Textures.end()) {
        const auto textureData = LoadBytes(filename);
        if (!textureData) {
            std::cerr << "Resources::GetTexture: Failed to load texture: " << filename << std::endl;
            return nullptr;
        }

        auto texture = std::make_shared<Paingine2D::Texture>();
        if (!texture->loadFromMemory(textureData->data(), textureData->size())) {
            std::cerr << "Resources::GetTexture: Failed to load texture from memory: " << filename << std::endl;
            return nullptr;
        }

        textureIt = g_Textures.emplace(filename, texture).first;
    }

    return textureIt->second;
}

std::shared_ptr<AudioClip> Resources::GetMusic(const std::string &filename) {
    auto bufferIt = g_AudioBuffers.find(filename);
    if (bufferIt == g_AudioBuffers.end()) {
        const auto musicData = LoadBytes(filename);
        if (!musicData) {
            std::cerr << "Resources::GetMusic: Failed to load music: " << filename << std::endl;
            return nullptr;
        }

        auto audioBuffer = AudioBuffer::CreateFromMemory(musicData->data(), musicData->size(), filename);
        if (!audioBuffer) {
            return nullptr;
        }

        bufferIt = g_AudioBuffers.emplace(filename, std::move(audioBuffer)).first;
    }

    return AudioClip::Create(bufferIt->second, filename);
}

std::shared_ptr<Paingine2D::Font> Resources::GetFont(const std::string &filename) {
    auto fontIt = g_Fonts.find(filename);
    if (fontIt == g_Fonts.end()) {
        const auto fontData = LoadBytes(filename);
        if (!fontData) {
            std::cerr << "Resources::GetFont: Failed to load font: " << filename << std::endl;
            return nullptr;
        }

        auto font = std::make_shared<Paingine2D::Font>();
        if (!font->loadFromMemory(fontData->data(), fontData->size())) {
            std::cerr << "Resources::GetFont: Failed to load font from memory: " << filename << std::endl;
            return nullptr;
        }

        g_FontBuffers[filename] = fontData;
        fontIt = g_Fonts.emplace(filename, font).first;
    }

    return fontIt->second;
}

std::vector<std::string> Resources::ListFiles() {
    std::vector<std::string> files;
    if (g_PakHandler != nullptr && !g_PakFile.empty()) {
        files = g_PakHandler->ListFiles(g_PakFile);
    }

    AssetFileSystem::AppendFiles(files, g_AssetRoot, "", g_FileSystemFallbackEnabled);
    std::sort(files.begin(), files.end());
    files.erase(std::unique(files.begin(), files.end()), files.end());
    return files;
}

std::vector<std::string> Resources::ListFilesWithPrefix(const std::string &prefix) {
    std::vector<std::string> files;
    if (g_PakHandler != nullptr && !g_PakFile.empty()) {
        files = g_PakHandler->ListFilesWithPrefix(g_PakFile, prefix);
    }

    AssetFileSystem::AppendFiles(files, g_AssetRoot, prefix, g_FileSystemFallbackEnabled);
    std::sort(files.begin(), files.end());
    files.erase(std::unique(files.begin(), files.end()), files.end());
    return files;
}

void Resources::SetPakker(Pakker *pakker) {
    g_PakHandler = pakker;
}
