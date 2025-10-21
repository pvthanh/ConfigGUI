// SPDX-License-Identifier: MIT
// FileUtils - File Utility Functions

#ifndef CONFIGGUI_UTILS_FILE_UTILS_H
#define CONFIGGUI_UTILS_FILE_UTILS_H

#include <string>
#include <vector>

namespace configgui {
namespace utils {

/**
 * @class FileUtils
 * @brief File system operations
 *
 * Common file operations: exists, extension, directory listing, etc.
 */
class FileUtils
{
public:
    /**
     * @brief Check if file exists
     * @param file_path Path to check
     * @return True if file exists
     */
    static bool exists(const std::string& file_path);

    /**
     * @brief Get file extension
     * @param file_path File path
     * @return Extension without dot (e.g., "json", "yaml")
     */
    static std::string getExtension(const std::string& file_path);

    /**
     * @brief Get directory from path
     * @param file_path File path
     * @return Directory path
     */
    static std::string getDirectory(const std::string& file_path);

    /**
     * @brief Get filename from path
     * @param file_path File path
     * @return Filename with extension
     */
    static std::string getFilename(const std::string& file_path);

    /**
     * @brief Get filename without extension
     * @param file_path File path
     * @return Filename without extension
     */
    static std::string getBasename(const std::string& file_path);

    /**
     * @brief Create directory (including parent directories)
     * @param dir_path Directory path
     * @return True if created or already exists
     */
    static bool createDirectory(const std::string& dir_path);

    /**
     * @brief List files in directory
     * @param dir_path Directory path
     * @param extension Optional extension filter (without dot)
     * @return Vector of file paths
     */
    static std::vector<std::string> listFiles(const std::string& dir_path,
                                               const std::string& extension = "");

    /**
     * @brief Read entire file into string
     * @param file_path File path
     * @return File contents as string
     */
    static std::string readFile(const std::string& file_path);

    /**
     * @brief Write string to file
     * @param file_path File path
     * @param content Content to write
     * @return True if successful
     */
    static bool writeFile(const std::string& file_path, const std::string& content);

private:
    FileUtils() = default;
};

} // namespace utils
} // namespace configgui

#endif // CONFIGGUI_UTILS_FILE_UTILS_H
