#include "Resources.h"

#include <algorithm>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <Paingine/Graphics/Font.hpp>
#include <Paingine/Graphics/Texture.hpp>

#include "Assets/AssetFileSystem.h"
#include "Audio/AudioClip.h"
#include "Utils/Log.h"
#include "Pak.h"

namespace {
    std::string g_PakFile;
    std::string g_AssetRoot = "Assets";
    bool g_FileSystemFallbackEnabled = true;
    Pakker *g_PakHandler = nullptr;

    std::map<std::string, std::shared_ptr<Paingine2D::Texture> > g_Textures;
    std::map<std::string, std::shared_ptr<AudioBuffer> > g_AudioBuffers;
    std::map<std::string, std::shared_ptr<Paingine2D::Font> > g_Fonts;

    std::shared_ptr<std::vector<uint8_t> > LoadPakAsset(const std::string &filename) {
        if (g_PakHandler == nullptr || g_PakFile.empty()) {
            return nullptr;
        }

        return g_PakHandler->LoadFile(g_PakFile, filename);
    }

    // Shared cache-miss path for asset types constructed via loadFromMemory.
    // Both Texture and Font copy the bytes they are given, so the source buffer
    // only has to outlive the call.
    template<typename T>
    std::shared_ptr<T> GetOrLoad(std::map<std::string, std::shared_ptr<T> > &cache,
                                 const std::string &filename,
                                 std::string_view scope) {
        const auto cached = cache.find(filename);
        if (cached != cache.end()) {
            return cached->second;
        }

        const auto bytes = Resources::LoadBytes(filename);
        if (!bytes) {
            Log::Error(scope, "failed to load " + filename);
            return nullptr;
        }

        auto asset = std::make_shared<T>();
        if (!asset->loadFromMemory(bytes->data(), bytes->size())) {
            Log::Error(scope, "failed to decode " + filename);
            return nullptr;
        }

        return cache.emplace(filename, std::move(asset)).first->second;
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
    return GetOrLoad(g_Textures, filename, "Resources::GetTexture");
}

std::shared_ptr<AudioClip> Resources::GetMusic(const std::string &filename) {
    auto bufferIt = g_AudioBuffers.find(filename);
    if (bufferIt == g_AudioBuffers.end()) {
        const auto musicData = LoadBytes(filename);
        if (!musicData) {
            Log::Error("Resources::GetMusic", "failed to load " + filename);
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
    return GetOrLoad(g_Fonts, filename, "Resources::GetFont");
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
