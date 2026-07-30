#pragma once

#include <string>
#include <filesystem>


struct FileProvider
{
    static std::filesystem::path findPathToFile(const std::string& filename) noexcept;
};
