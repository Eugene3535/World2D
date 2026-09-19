#include <cassert>

#ifdef _WIN32
#include <Windows.h>
#endif

#ifdef __linux__
#include <unistd.h>
#endif

#include "files/FileProvider.hpp"


static FileProvider* s_instance;

static std::filesystem::path get_root_path() noexcept
{
#ifdef _WIN32
    wchar_t buffer[MAX_PATH]{};
    DWORD result = GetModuleFileNameW(nullptr, buffer, MAX_PATH);

    if (result == 0) 
        return {};

    int size = WideCharToMultiByte(CP_UTF8, 0, buffer, -1, nullptr, 0, nullptr, nullptr);

    if (size == 0) 
        return {};

    std::string path(size, 0);
    size = WideCharToMultiByte(CP_UTF8, 0, buffer, -1, path.data(), size, nullptr, nullptr);

    if (size == 0) 
        return {};

    size_t pos = path.find_last_of("/\\");

    if (pos != std::string::npos)
        path.resize(pos);

    return std::filesystem::path(path);

#endif // !_WIN32

#ifdef __linux__
    std::string buffer(256, '\0');
    ssize_t count = readlink("/proc/self/exe", buffer.data(), buffer.size());

    if (count != -1) 
    {
        size_t pos = buffer.find_last_of('/');

        if (pos != std::string::npos)
            buffer.resize(pos);

        return std::filesystem::path(buffer);
    }
#endif  // !__linux__

    return {};
}


FileProvider::FileProvider(const char* argv) noexcept
{
    assert(s_instance == nullptr);
    s_instance = this;

    std::error_code ec;
    std::filesystem::path exePath = std::filesystem::canonical(argv, ec);

    if (!ec)
        m_exeDir = exePath.parent_path();
    else
        m_exeDir = get_root_path();
}


std::filesystem::path FileProvider::findPathToFile(const std::string& filename) noexcept
{
    if (s_instance)
    {
        const std::filesystem::path resFolder = s_instance->m_exeDir / "res";

        if (std::filesystem::exists(resFolder))
        {
            for (const auto& file : std::filesystem::recursive_directory_iterator(resFolder))
                if (file.path().filename() == filename)    
                    return file.path();            
        }
    }

    return std::filesystem::path();
}