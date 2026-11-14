#pragma once

#include <filesystem>
#include <string>


class FileProvider
{
public:
    explicit FileProvider(const char* argv) noexcept;

    static std::filesystem::path        findPathToFile(const std::string& filename) noexcept;
    static const std::filesystem::path& getAppPath() noexcept;

private:
    std::filesystem::path m_exeDir;
};