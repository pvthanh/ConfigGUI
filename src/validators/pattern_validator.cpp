// SPDX-License-Identifier: MIT
// PatternValidator - Implementation

#include "pattern_validator.h"

namespace configgui {
namespace validators {

// Initialize static members
mutable std::shared_mutex PatternValidator::s_regex_cache_mutex;
std::unordered_map<std::string, std::shared_ptr<const std::regex>> PatternValidator::s_regex_cache;
size_t PatternValidator::s_cache_lookups = 0;

ValidationResult PatternValidator::validate(const json& value, const json& schema)
{
    if (!value.is_string() || !schema.contains("pattern"))
    {
        return success();
    }

    if (!schema["pattern"].is_string())
    {
        return success();
    }

    const auto str = value.get<std::string>();
    const auto pattern = schema["pattern"].get<std::string>();

    if (!matchesPattern(str, pattern))
    {
        std::string message = "String does not match pattern: " + pattern;
        return failure({makeError("value", message, "PATTERN_MISMATCH")});
    }

    return success();
}

bool PatternValidator::matchesPattern(const std::string& str, const std::string& pattern) const
{
    try
    {
        auto regex_ptr = getCachedRegex(pattern);
        return std::regex_match(str, *regex_ptr);
    }
    catch (const std::regex_error&)
    {
        // Invalid regex pattern - return true to not fail validation on bad pattern
        return true;
    }
}

std::shared_ptr<const std::regex> PatternValidator::getCachedRegex(const std::string& pattern)
{
    // Increment lookup counter (for statistics)
    ++s_cache_lookups;

    // Try to read from cache first (shared lock)
    {
        std::shared_lock<std::shared_mutex> read_lock(s_regex_cache_mutex);
        auto it = s_regex_cache.find(pattern);
        if (it != s_regex_cache.end())
        {
            return it->second;
        }
    }

    // Pattern not in cache, compile and store it (exclusive lock)
    std::unique_lock<std::shared_mutex> write_lock(s_regex_cache_mutex);

    // Double-check pattern wasn't inserted by another thread
    auto it = s_regex_cache.find(pattern);
    if (it != s_regex_cache.end())
    {
        return it->second;
    }

    // Compile and cache the new pattern
    auto compiled_regex = std::make_shared<const std::regex>(pattern);
    s_regex_cache[pattern] = compiled_regex;

    return compiled_regex;
}

void PatternValidator::clearCache()
{
    std::unique_lock<std::shared_mutex> lock(s_regex_cache_mutex);
    s_regex_cache.clear();
    s_cache_lookups = 0;
}

std::pair<size_t, size_t> PatternValidator::getCacheStats()
{
    std::shared_lock<std::shared_mutex> lock(s_regex_cache_mutex);
    return {s_regex_cache.size(), s_cache_lookups};
}

} // namespace validators
} // namespace configgui

