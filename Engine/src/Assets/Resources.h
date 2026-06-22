#pragma once

#include <memory>
#include <string>
#include <vector>

class AudioClip;
class Pakker;

namespace Paingine2D {
class Font;
class Texture;
}

class Resources {
public:
    // Resources currently binds one Pak file at a time.
    static void BindPakFile(const std::string &pakFilename);
    static void SetAssetRoot(const std::string &rootDirectory);
    static const std::string &GetAssetRoot();
    static void SetFileSystemFallbackEnabled(bool enabled);

    static void Unload();

    static std::shared_ptr<std::vector<uint8_t> > LoadBytes(const std::string &filename);

    static std::shared_ptr<Paingine2D::Texture> GetTexture(const std::string &filename);

    static std::shared_ptr<AudioClip> GetMusic(const std::string &filename);

    static std::shared_ptr<Paingine2D::Font> GetFont(const std::string &filename);

    static std::vector<std::string> ListFiles();

    static std::vector<std::string> ListFilesWithPrefix(const std::string &prefix);

    static void SetPakker(Pakker *pakker);
};
