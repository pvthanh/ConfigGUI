// SPDX-License-Identifier: MIT
// StringUtils - String Utility Functions

#ifndef CONFIGGUI_UTILS_STRING_UTILS_H
#define CONFIGGUI_UTILS_STRING_UTILS_H

#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

namespace configgui {
namespace utils {

/**
 * @class StringUtils
 * @brief String manipulation utilities
 *
 * Common string operations: trim, split, case conversion, etc.
 */
class StringUtils
{
public:
    /**
     * @brief Trim whitespace from start and end
     * @param str String to trim
     * @return Trimmed string
     */
    static std::string trim(const std::string& str);

    /**
     * @brief Trim whitespace from start
     * @param str String to trim
     * @return Left-trimmed string
     */
    static std::string ltrim(const std::string& str);

    /**
     * @brief Trim whitespace from end
     * @param str String to trim
     * @return Right-trimmed string
     */
    static std::string rtrim(const std::string& str);

    /**
     * @brief Split string by delimiter
     * @param str String to split
     * @param delimiter Delimiter character
     * @return Vector of substrings
     */
    static std::vector<std::string> split(const std::string& str, char delimiter);

    /**
     * @brief Convert string to lowercase
     * @param str String to convert
     * @return Lowercase string
     */
    static std::string toLower(const std::string& str);

    /**
     * @brief Convert string to uppercase
     * @param str String to convert
     * @return Uppercase string
     */
    static std::string toUpper(const std::string& str);

    /**
     * @brief Check if string starts with prefix
     * @param str String to check
     * @param prefix Prefix to match
     * @return True if starts with prefix
     */
    static bool startsWith(const std::string& str, const std::string& prefix);

    /**
     * @brief Check if string ends with suffix
     * @param str String to check
     * @param suffix Suffix to match
     * @return True if ends with suffix
     */
    static bool endsWith(const std::string& str, const std::string& suffix);

    /**
     * @brief Replace all occurrences in string
     * @param str String to process
     * @param from Pattern to find
     * @param to Replacement text
     * @return String with replacements
     */
    static std::string replace(const std::string& str, const std::string& from,
                               const std::string& to);

private:
    StringUtils() = default;
};

} // namespace utils
} // namespace configgui

#endif // CONFIGGUI_UTILS_STRING_UTILS_H
