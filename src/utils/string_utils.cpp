// SPDX-License-Identifier: MIT
// StringUtils - Implementation

#include "string_utils.h"

namespace configgui {
namespace utils {

std::string StringUtils::trim(const std::string& str)
{
    return rtrim(ltrim(str));
}

std::string StringUtils::ltrim(const std::string& str)
{
    size_t start = 0;
    while (start < str.length() && std::isspace(str[start]))
    {
        ++start;
    }
    return str.substr(start);
}

std::string StringUtils::rtrim(const std::string& str)
{
    size_t end = str.length();
    while (end > 0 && std::isspace(str[end - 1]))
    {
        --end;
    }
    return str.substr(0, end);
}

std::vector<std::string> StringUtils::split(const std::string& str, char delimiter)
{
    std::vector<std::string> result;
    std::string current;

    // OPTIMIZATION: Pre-allocate based on delimiter count estimate
    // Count delimiters to estimate number of parts (typical +1 for last part)
    size_t delimiter_count = static_cast<size_t>(std::count(str.begin(), str.end(), delimiter));
    result.reserve(delimiter_count + 1);

    // Pre-allocate current string to avoid repeated allocations during concatenation
    current.reserve(str.length() / std::max(delimiter_count, size_t(1)));

    for (char c : str)
    {
        if (c == delimiter)
        {
            result.push_back(current);
            current.clear();
        }
        else
        {
            current += c;
        }
    }

    if (!current.empty())
    {
        result.push_back(current);
    }

    return result;
}

std::string StringUtils::toLower(const std::string& str)
{
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

std::string StringUtils::toUpper(const std::string& str)
{
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return result;
}

bool StringUtils::startsWith(const std::string& str, const std::string& prefix)
{
    if (prefix.length() > str.length())
    {
        return false;
    }
    return str.compare(0, prefix.length(), prefix) == 0;
}

bool StringUtils::endsWith(const std::string& str, const std::string& suffix)
{
    if (suffix.length() > str.length())
    {
        return false;
    }
    return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}

std::string StringUtils::replace(const std::string& str, const std::string& from,
                                 const std::string& to)
{
    if (from.empty())
    {
        return str;
    }

    std::string result = str;
    size_t pos = 0;

    while ((pos = result.find(from, pos)) != std::string::npos)
    {
        result.replace(pos, from.length(), to);
        pos += to.length();
    }

    return result;
}

} // namespace utils
} // namespace configgui
