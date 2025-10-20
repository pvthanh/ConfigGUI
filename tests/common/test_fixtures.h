// SPDX-License-Identifier: MIT
// Common Test Fixtures and Utilities

#ifndef TESTS_COMMON_TEST_FIXTURES_H
#define TESTS_COMMON_TEST_FIXTURES_H

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>

using json = nlohmann::json;
namespace fs = std::filesystem;

/**
 * @class BaseTestFixture
 * @brief Common setup/teardown for all tests
 */
class BaseTestFixture : public ::testing::Test
{
protected:
    virtual void SetUp() override
    {
        // Create temporary directory for test files
        test_temp_dir_ = fs::temp_directory_path() / "configgui_tests";
        fs::create_directories(test_temp_dir_);
    }

    virtual void TearDown() override
    {
        // Clean up temporary files
        try
        {
            fs::remove_all(test_temp_dir_);
        }
        catch (const std::exception&)
        {
            // Ignore cleanup errors
        }
    }

    /**
     * @brief Create a temporary file for testing
     * @param filename Name of file to create
     * @param content Content to write
     * @return Full path to temporary file
     */
    std::string createTempFile(const std::string& filename, const std::string& content)
    {
        auto file_path = test_temp_dir_ / filename;
        std::ofstream file(file_path);
        file << content;
        file.close();
        return file_path.string();
    }

    /**
     * @brief Create a temporary JSON file
     * @param filename Name of file
     * @param json_data JSON content
     * @return Full path to temporary file
     */
    std::string createTempJsonFile(const std::string& filename, const json& json_data)
    {
        return createTempFile(filename, json_data.dump(4));
    }

    /**
     * @brief Read a file as string
     * @param file_path Path to file
     * @return File content as string
     */
    std::string readFile(const std::string& file_path)
    {
        std::ifstream file(file_path);
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    /**
     * @brief Parse JSON from file
     * @param file_path Path to JSON file
     * @return Parsed JSON object
     */
    json readJsonFile(const std::string& file_path)
    {
        std::ifstream file(file_path);
        json data;
        file >> data;
        return data;
    }

    /**
     * @brief Check if file exists
     * @param file_path Path to check
     * @return True if file exists
     */
    bool fileExists(const std::string& file_path) const
    {
        return fs::exists(file_path);
    }

    fs::path test_temp_dir_;
};

/**
 * @class ValidatorTestFixture
 * @brief Setup for validator tests
 */
class ValidatorTestFixture : public BaseTestFixture
{
protected:
    virtual void SetUp() override
    {
        BaseTestFixture::SetUp();

        // Create sample schemas
        string_schema_ = {
            {"type", "string"},
            {"minLength", 1},
            {"maxLength", 100}
        };

        integer_schema_ = {
            {"type", "integer"},
            {"minimum", 0},
            {"maximum", 100}
        };

        enum_schema_ = {
            {"type", "string"},
            {"enum", json::array({"dev", "staging", "prod"})}
        };

        pattern_schema_ = {
            {"type", "string"},
            {"pattern", "^[A-Z][a-z]*$"}
        };
    }

    json string_schema_;
    json integer_schema_;
    json enum_schema_;
    json pattern_schema_;
};

/**
 * @class FileIOTestFixture
 * @brief Setup for file I/O tests
 */
class FileIOTestFixture : public BaseTestFixture
{
protected:
    virtual void SetUp() override
    {
        BaseTestFixture::SetUp();

        // Create sample configuration
        sample_config_ = {
            {"name", "Test Config"},
            {"version", 1},
            {"settings", {
                {"debug", true},
                {"timeout", 30},
                {"hosts", json::array({"localhost", "127.0.0.1"})}
            }}
        };
    }

    json sample_config_;
};

/**
 * @class SchemaTestFixture
 * @brief Setup for schema tests
 */
class SchemaTestFixture : public BaseTestFixture
{
protected:
    virtual void SetUp() override
    {
        BaseTestFixture::SetUp();

        // Create comprehensive schema
        schema_ = {
            {"$schema", "http://json-schema.org/draft-07/schema#"},
            {"type", "object"},
            {"properties", {
                {"name", {
                    {"type", "string"},
                    {"minLength", 1}
                }},
                {"age", {
                    {"type", "integer"},
                    {"minimum", 0},
                    {"maximum", 150}
                }},
                {"email", {
                    {"type", "string"},
                    {"pattern", "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$"}
                }},
                {"role", {
                    {"type", "string"},
                    {"enum", json::array({"admin", "user", "guest"})}
                }}
            }},
            {"required", json::array({"name", "email"})}
        };
    }

    json schema_;
};

/**
 * @brief Create a valid test configuration
 * @return JSON configuration object
 */
inline json createValidConfig()
{
    return {
        {"name", "John Doe"},
        {"age", 30},
        {"email", "john@example.com"},
        {"role", "user"}
    };
}

/**
 * @brief Create an invalid test configuration
 * @return JSON configuration object with validation errors
 */
inline json createInvalidConfig()
{
    return {
        {"age", -5},  // Invalid age
        {"email", "invalid-email"},  // Invalid email format
        {"role", "superuser"}  // Invalid role
    };
}

/**
 * @brief Assert that a ValidationResult is valid
 * @param result Result to check
 */
inline void ASSERT_VALID_RESULT(const auto& result)
{
    ASSERT_TRUE(result.is_valid) << "Expected valid result but got errors: "
                                 << result.errors.size();
    for (const auto& error : result.errors)
    {
        ADD_FAILURE() << error.field << ": " << error.message;
    }
}

/**
 * @brief Assert that a ValidationResult has errors
 * @param result Result to check
 * @param expected_error_count Expected number of errors
 */
inline void ASSERT_INVALID_RESULT(const auto& result, size_t expected_error_count = 0)
{
    ASSERT_FALSE(result.is_valid) << "Expected invalid result but validation passed";
    if (expected_error_count > 0)
    {
        ASSERT_EQ(result.errors.size(), expected_error_count);
    }
}

#endif // TESTS_COMMON_TEST_FIXTURES_H
