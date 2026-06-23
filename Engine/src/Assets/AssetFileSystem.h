#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace AssetFileSystem {
    std::shared_ptr<std::vector<uint8_t> > LoadBytes(const std::string &assetRoot,
                                                     const std::string &filename,
                                                     bool fallbackEnabled);

    void AppendFiles(std::vector<std::string> &files,
                     const std::string &assetRoot,
                     const std::string &prefix,
                     bool fallbackEnabled);
}
