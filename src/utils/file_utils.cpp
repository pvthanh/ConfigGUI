// SPDX-License-Identifier: MIT
// FileUtils - Implementation

#include "file_utils.h"
#include <filesystem>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

namespace configgui {
namespace utils {

bool FileUtils::exists(const std::string& file_path)
{
    return fs::exists(file_path);
}

std::string FileUtils::getExtension(const std::string& file_path)
{
    const auto path = fs::path(file_path);
    auto ext = path.extension().string();

    if (!ext.empty() && ext[0] == '.')
    {
        ext = ext.substr(1);
    }

    return ext;
}

std::string FileUtils::getDirectory(const std::string& file_path)
{
    const auto path = fs::path(file_path);
    return path.parent_path().string();
}

std::string FileUtils::getFilename(const std::string& file_path)
{
    const auto path = fs::path(file_path);
    return path.filename().string();
}

std::string FileUtils::getBasename(const std::string& file_path)
{
    const auto path = fs::path(file_path);
    return path.stem().string();
}

bool FileUtils::createDirectory(const std::string& dir_path)
{
    try
    {
        fs::create_directories(dir_path);
        return true;
    }
    catch (const std::exception&)
    {
        return false;
    }
}

std::vector<std::string> FileUtils::listFiles(const std::string& dir_path,
                                               const std::string& extension)
{
    std::vector<std::string> files;

    try
    {
        if (!fs::exists(dir_path) || !fs::is_directory(dir_path))
        {
            return files;
        }

        for (const auto& entry : fs::directory_iterator(dir_path))
        {
            if (entry.is_regular_file())
            {
                if (extension.empty())
                {
                    files.push_back(entry.path().string());
                }
                else
                {
                    const auto file_ext = entry.path().extension().string();
                    if (file_ext.length() > 1 && file_ext.substr(1) == extension)
                    {
                        files.push_back(entry.path().string());
                    }
                }
            }
        }
    }
    catch (const std::exception&)
    {
        // Return empty list on error
    }

    return files;
}

std::string FileUtils::readFile(const std::string& file_path)
{
    try
    {
        std::ifstream file(file_path);
        if (!file.is_open())
        {
            return "";
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();

        return buffer.str();
    }
    catch (const std::exception&)
    {
        return "";
    }
}

bool FileUtils::writeFile(const std::string& file_path, const std::string& content)
{
    try
    {
        std::ofstream file(file_path);
        if (!file.is_open())
        {
            return false;
        }

        file << content;
        file.close();

        return true;
    }
    catch (const std::exception&)
    {
        return false;
    }
}

} // namespace utils
} // namespace configgui
