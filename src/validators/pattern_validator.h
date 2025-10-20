// SPDX-License-Identifier: MIT
// PatternValidator - Pattern Validation

#ifndef CONFIGGUI_VALIDATORS_PATTERN_VALIDATOR_H
#define CONFIGGUI_VALIDATORS_PATTERN_VALIDATOR_H

#include "ivalidator.h"
#include <regex>
#include <unordered_map>
#include <shared_mutex>
#include <memory>

namespace configgui {
namespace validators {

/**
 * @class PatternValidator
 * @brief Validates string patterns with regex with pattern caching
 *
 * Checks that string values match regex patterns.
 * Supports "pattern" constraint from JSON Schema.
 *
 * OPTIMIZATION: Caches compiled regex patterns for reuse.
 * Expected improvement: 10-50x faster for repeated patterns.
 */
class PatternValidator : public IValidator
{
public:
    ~PatternValidator() override = default;

    /**
     * @brief Validate string pattern
     * @param value String value to check
     * @param schema Schema with pattern constraint
     * @return ValidationResult
     */
    ValidationResult validate(const json& value, const json& schema) override;

    /**
     * @brief Get validator name
     * @return "PatternValidator"
     */
    std::string getName() const override { return "PatternValidator"; }

    /**
     * @brief Clear the regex pattern cache
     * Useful for memory management or test cleanup
     */
    static void clearCache();

    /**
     * @brief Get cache statistics for performance analysis
     * @return pair<cached_patterns_count, total_lookups>
     */
    static std::pair<size_t, size_t> getCacheStats();

private:
    /**
     * @brief Check if string matches pattern with cached regex
     * @param str String to match
     * @param pattern Regex pattern
     * @return True if matches
     */
    bool matchesPattern(const std::string& str, const std::string& pattern) const;

    /**
     * @brief Get or compile regex pattern from cache
     * Thread-safe access to cached patterns
     * @param pattern Pattern string to compile
     * @return Shared pointer to compiled regex
     */
    static std::shared_ptr<const std::regex> getCachedRegex(const std::string& pattern);

    // Static thread-safe regex cache
    static mutable std::shared_mutex s_regex_cache_mutex;
    static std::unordered_map<std::string, std::shared_ptr<const std::regex>> s_regex_cache;
    static size_t s_cache_lookups;
};

} // namespace validators
} // namespace configgui

#endif // CONFIGGUI_VALIDATORS_PATTERN_VALIDATOR_H
