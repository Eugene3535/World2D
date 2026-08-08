#include <cassert>
#include <system_error>

#include "files/FileProvider.hpp"


static FileProvider* s_instance;


FileProvider::FileProvider(const char* argv) noexcept
{
    assert(s_instance == nullptr);
    s_instance = this;

    std::error_code ec;
    std::filesystem::path exePath = std::filesystem::canonical(argv, ec);

    if (!ec)
        m_exeDir = exePath.parent_path();
    else
        m_exeDir = std::filesystem::current_path();
}


std::filesystem::path FileProvider::findPathToFile(const std::string& filename) noexcept
{
    std::filesystem::path resFolder = s_instance->m_exeDir / "res";

    if(std::filesystem::exists(resFolder))
    {
        for (const auto& file : std::filesystem::recursive_directory_iterator(resFolder))
            if (file.path().filename() == filename)    
                return file.path();            
    }

    return std::filesystem::path();
}