#include "AssetFileSystem.h"

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <system_error>

namespace {
    std::string NormalizeAssetPath(std::filesystem::path path) {
        return path.generic_string();
    }

    std::filesystem::path ResolveAssetPath(const std::string &assetRoot, const std::string &filename) {
        const std::filesystem::path requestedPath(filename);
        if (requestedPath.is_absolute()) {
            return requestedPath;
        }

        const std::filesystem::path rootedPath = std::filesystem::path(assetRoot) / requestedPath;
        if (std::filesystem::exists(rootedPath)) {
            return rootedPath;
        }

        return requestedPath;
    }
}

namespace AssetFileSystem {
    std::shared_ptr<std::vector<uint8_t> > LoadBytes(const std::string &assetRoot,
                                                     const std::string &filename,
                                                     bool fallbackEnabled) {
        if (!fallbackEnabled) {
            return nullptr;
        }

        const std::filesystem::path path = ResolveAssetPath(assetRoot, filename);
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file) {
            return nullptr;
        }

        const std::streamsize size = file.tellg();
        if (size < 0) {
            return nullptr;
        }

        auto data = std::make_shared<std::vector<uint8_t> >(static_cast<size_t>(size));
        file.seekg(0, std::ios::beg);
        if (size > 0 && !file.read(reinterpret_cast<char *>(data->data()), size)) {
            return nullptr;
        }

        return data;
    }

    void AppendFiles(std::vector<std::string> &files,
                     const std::string &assetRoot,
                     const std::string &prefix,
                     bool fallbackEnabled) {
        if (!fallbackEnabled) {
            return;
        }

        const std::filesystem::path root(assetRoot);
        std::error_code error;
        if (!std::filesystem::exists(root, error) || error) {
            return;
        }

        constexpr auto options = std::filesystem::directory_options::skip_permission_denied;
        for (std::filesystem::recursive_directory_iterator it(root, options, error), end; it != end; it.increment(error)) {
            if (error) {
                error.clear();
                continue;
            }

            const std::filesystem::directory_entry &entry = *it;
            if (!entry.is_regular_file(error) || error) {
                error.clear();
                continue;
            }

            const auto relative = NormalizeAssetPath(std::filesystem::relative(entry.path(), root, error));
            if (error) {
                error.clear();
                continue;
            }

            if (prefix.empty() || relative.starts_with(prefix)) {
                files.push_back(relative);
            }
        }
    }
}
