#pragma once

#include <string>
#include <string_view>
#include <filesystem>
#include <vector>


class FileProvider
{
public:
    explicit FileProvider(const char* argv) noexcept;

    static std::filesystem::path findPathToFile(const std::string& filename) noexcept;

private:
    std::filesystem::path m_exeDir;
};
