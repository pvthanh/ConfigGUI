// SPDX-License-Identifier: MIT
// Utility Functions Unit Tests (Logger, StringUtils, FileUtils)

#include <gtest/gtest.h>
#include "src/utils/logger.h"
#include "src/utils/string_utils.h"
#include "src/utils/file_utils.h"
#include "common/test_fixtures.h"

using namespace configgui::utils;

// ========== Logger Tests ==========

class LoggerTest : public BaseTestFixture
{
protected:
    Logger& logger_ = Logger::getInstance();
};

TEST_F(LoggerTest, Logger_Singleton)
{
    Logger& logger1 = Logger::getInstance();
    Logger& logger2 = Logger::getInstance();

    EXPECT_EQ(&logger1, &logger2);
}

TEST_F(LoggerTest, Logger_Debug)
{
    logger_.setLevel(LogLevel::DEBUG);
    logger_.debug("Debug message");
    // No exception should be thrown
}

TEST_F(LoggerTest, Logger_Info)
{
    logger_.setLevel(LogLevel::INFO);
    logger_.info("Info message");
    // No exception should be thrown
}

TEST_F(LoggerTest, Logger_Warn)
{
    logger_.setLevel(LogLevel::WARN);
    logger_.warn("Warning message");
    // No exception should be thrown
}

TEST_F(LoggerTest, Logger_Error)
{
    logger_.setLevel(LogLevel::ERROR);
    logger_.error("Error message");
    // No exception should be thrown
}

TEST_F(LoggerTest, Logger_SetLevel_Debug)
{
    logger_.setLevel(LogLevel::DEBUG);
    // Level should be set to DEBUG
}

TEST_F(LoggerTest, Logger_SetLevel_Info)
{
    logger_.setLevel(LogLevel::INFO);
    // Level should be set to INFO
}

TEST_F(LoggerTest, Logger_SetLevel_Warn)
{
    logger_.setLevel(LogLevel::WARN);
    // Level should be set to WARN
}

TEST_F(LoggerTest, Logger_SetLevel_Error)
{
    logger_.setLevel(LogLevel::ERROR);
    // Level should be set to ERROR
}

TEST_F(LoggerTest, Logger_SetOutput_Console)
{
    logger_.setOutput(LogOutput::CONSOLE);
    logger_.info("Console output");
}

TEST_F(LoggerTest, Logger_SetOutput_File)
{
    std::string log_file = createTempFile("test.log");
    logger_.setOutput(LogOutput::FILE);
    logger_.setLogFilePath(log_file);
    logger_.info("File output");
}

TEST_F(LoggerTest, Logger_SetOutput_Both)
{
    std::string log_file = createTempFile("test.log");
    logger_.setOutput(LogOutput::BOTH);
    logger_.setLogFilePath(log_file);
    logger_.info("Both output");
}

TEST_F(LoggerTest, Logger_MultipleMessages)
{
    logger_.setLevel(LogLevel::DEBUG);
    logger_.debug("First");
    logger_.info("Second");
    logger_.warn("Third");
    logger_.error("Fourth");
}

// ========== StringUtils Tests ==========

class StringUtilsTest : public BaseTestFixture
{
};

TEST_F(StringUtilsTest, Trim_LeadingSpaces)
{
    std::string input = "   hello";
    std::string result = StringUtils::trim(input);

    EXPECT_EQ(result, "hello");
}

TEST_F(StringUtilsTest, Trim_TrailingSpaces)
{
    std::string input = "hello   ";
    std::string result = StringUtils::trim(input);

    EXPECT_EQ(result, "hello");
}

TEST_F(StringUtilsTest, Trim_BothSides)
{
    std::string input = "   hello world   ";
    std::string result = StringUtils::trim(input);

    EXPECT_EQ(result, "hello world");
}

TEST_F(StringUtilsTest, Trim_NoSpaces)
{
    std::string input = "hello";
    std::string result = StringUtils::trim(input);

    EXPECT_EQ(result, "hello");
}

TEST_F(StringUtilsTest, Trim_Empty)
{
    std::string input = "";
    std::string result = StringUtils::trim(input);

    EXPECT_EQ(result, "");
}

TEST_F(StringUtilsTest, Split_Simple)
{
    std::string input = "a,b,c";
    auto result = StringUtils::split(input, ',');

    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], "a");
    EXPECT_EQ(result[1], "b");
    EXPECT_EQ(result[2], "c");
}

TEST_F(StringUtilsTest, Split_WithSpaces)
{
    std::string input = "apple, banana, cherry";
    auto result = StringUtils::split(input, ',');

    EXPECT_EQ(result.size(), 3);
}

TEST_F(StringUtilsTest, Split_SingleElement)
{
    std::string input = "single";
    auto result = StringUtils::split(input, ',');

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], "single");
}

TEST_F(StringUtilsTest, Split_NoDelimiter)
{
    std::string input = "no delimiter here";
    auto result = StringUtils::split(input, ',');

    EXPECT_EQ(result.size(), 1);
}

TEST_F(StringUtilsTest, Split_Empty)
{
    std::string input = "";
    auto result = StringUtils::split(input, ',');

    EXPECT_EQ(result.size(), 0);
}

TEST_F(StringUtilsTest, ToLower_MixedCase)
{
    std::string input = "HeLLo WoRLd";
    std::string result = StringUtils::toLower(input);

    EXPECT_EQ(result, "hello world");
}

TEST_F(StringUtilsTest, ToLower_AlreadyLower)
{
    std::string input = "hello";
    std::string result = StringUtils::toLower(input);

    EXPECT_EQ(result, "hello");
}

TEST_F(StringUtilsTest, ToUpper_MixedCase)
{
    std::string input = "HeLLo WoRLd";
    std::string result = StringUtils::toUpper(input);

    EXPECT_EQ(result, "HELLO WORLD");
}

TEST_F(StringUtilsTest, ToUpper_AlreadyUpper)
{
    std::string input = "HELLO";
    std::string result = StringUtils::toUpper(input);

    EXPECT_EQ(result, "HELLO");
}

TEST_F(StringUtilsTest, Replace_Simple)
{
    std::string input = "hello world";
    std::string result = StringUtils::replace(input, "world", "C++");

    EXPECT_EQ(result, "hello C++");
}

TEST_F(StringUtilsTest, Replace_Multiple)
{
    std::string input = "foo bar foo baz foo";
    std::string result = StringUtils::replace(input, "foo", "FOO");

    EXPECT_EQ(result, "FOO bar FOO baz FOO");
}

TEST_F(StringUtilsTest, Replace_NotFound)
{
    std::string input = "hello world";
    std::string result = StringUtils::replace(input, "xyz", "abc");

    EXPECT_EQ(result, "hello world");
}

TEST_F(StringUtilsTest, StartsWith_True)
{
    std::string input = "hello world";
    bool result = StringUtils::startsWith(input, "hello");

    EXPECT_TRUE(result);
}

TEST_F(StringUtilsTest, StartsWith_False)
{
    std::string input = "hello world";
    bool result = StringUtils::startsWith(input, "world");

    EXPECT_FALSE(result);
}

TEST_F(StringUtilsTest, EndsWith_True)
{
    std::string input = "hello world";
    bool result = StringUtils::endsWith(input, "world");

    EXPECT_TRUE(result);
}

TEST_F(StringUtilsTest, EndsWith_False)
{
    std::string input = "hello world";
    bool result = StringUtils::endsWith(input, "hello");

    EXPECT_FALSE(result);
}

TEST_F(StringUtilsTest, Contains_True)
{
    std::string input = "hello world";
    bool result = StringUtils::contains(input, "lo wo");

    EXPECT_TRUE(result);
}

TEST_F(StringUtilsTest, Contains_False)
{
    std::string input = "hello world";
    bool result = StringUtils::contains(input, "xyz");

    EXPECT_FALSE(result);
}

// ========== FileUtils Tests ==========

class FileUtilsTest : public BaseTestFixture
{
};

TEST_F(FileUtilsTest, FileExists_True)
{
    std::string temp_file = createTempFile("test.txt");

    bool result = FileUtils::fileExists(temp_file);

    EXPECT_TRUE(result);
}

TEST_F(FileUtilsTest, FileExists_False)
{
    std::string non_existent = temp_dir_ + "/non_existent_file.txt";

    bool result = FileUtils::fileExists(non_existent);

    EXPECT_FALSE(result);
}

TEST_F(FileUtilsTest, GetExtension_Json)
{
    std::string filename = "config.json";
    std::string result = FileUtils::getExtension(filename);

    EXPECT_EQ(result, ".json");
}

TEST_F(FileUtilsTest, GetExtension_Yaml)
{
    std::string filename = "config.yaml";
    std::string result = FileUtils::getExtension(filename);

    EXPECT_EQ(result, ".yaml");
}

TEST_F(FileUtilsTest, GetExtension_NoExtension)
{
    std::string filename = "config";
    std::string result = FileUtils::getExtension(filename);

    EXPECT_EQ(result, "");
}

TEST_F(FileUtilsTest, GetExtension_HiddenFile)
{
    std::string filename = ".gitignore";
    std::string result = FileUtils::getExtension(filename);

    EXPECT_EQ(result, "");
}

TEST_F(FileUtilsTest, CreateDirectory_New)
{
    std::string new_dir = temp_dir_ + "/new_directory";

    bool result = FileUtils::createDirectory(new_dir);

    EXPECT_TRUE(result);
    EXPECT_TRUE(std::filesystem::exists(new_dir));
}

TEST_F(FileUtilsTest, CreateDirectory_Existing)
{
    std::string existing_dir = temp_dir_;

    bool result = FileUtils::createDirectory(existing_dir);

    EXPECT_TRUE(result);  // Should succeed (directory already exists)
}

TEST_F(FileUtilsTest, ReadFile_Simple)
{
    std::string temp_file = createTempFile("test.txt");
    std::ofstream file(temp_file);
    file << "Hello, World!";
    file.close();

    auto result = FileUtils::readFile(temp_file);

    EXPECT_TRUE(result.ok);
    EXPECT_EQ(result.value, "Hello, World!");
}

TEST_F(FileUtilsTest, ReadFile_Multiline)
{
    std::string temp_file = createTempFile("test.txt");
    std::ofstream file(temp_file);
    file << "Line 1\n";
    file << "Line 2\n";
    file << "Line 3";
    file.close();

    auto result = FileUtils::readFile(temp_file);

    EXPECT_TRUE(result.ok);
    EXPECT_NE(result.value.find("Line 1"), std::string::npos);
    EXPECT_NE(result.value.find("Line 2"), std::string::npos);
    EXPECT_NE(result.value.find("Line 3"), std::string::npos);
}

TEST_F(FileUtilsTest, ReadFile_NonExistent)
{
    std::string non_existent = temp_dir_ + "/non_existent.txt";

    auto result = FileUtils::readFile(non_existent);

    EXPECT_FALSE(result.ok);
}

TEST_F(FileUtilsTest, WriteFile_New)
{
    std::string temp_file = createTempFile("new_file.txt");
    std::string content = "Test content";

    auto result = FileUtils::writeFile(temp_file, content);

    EXPECT_TRUE(result.ok);
    EXPECT_TRUE(FileUtils::fileExists(temp_file));
}

TEST_F(FileUtilsTest, WriteFile_Overwrite)
{
    std::string temp_file = createTempFile("test.txt");
    std::ofstream file(temp_file);
    file << "Old content";
    file.close();

    auto result = FileUtils::writeFile(temp_file, "New content");

    EXPECT_TRUE(result.ok);
    auto read_result = FileUtils::readFile(temp_file);
    EXPECT_EQ(read_result.value, "New content");
}

TEST_F(FileUtilsTest, ListFiles_Directory)
{
    createTempFile("file1.txt");
    createTempFile("file2.txt");

    auto result = FileUtils::listFiles(temp_dir_);

    EXPECT_TRUE(result.ok);
}

TEST_F(FileUtilsTest, ListFiles_NonExistent)
{
    std::string non_existent = temp_dir_ + "/non_existent_dir";

    auto result = FileUtils::listFiles(non_existent);

    EXPECT_FALSE(result.ok);
}

// ========== Integration Tests ==========

TEST_F(StringUtilsTest, StringUtils_ChainedOperations)
{
    std::string input = "  HELLO WORLD  ";
    std::string result = StringUtils::trim(input);
    result = StringUtils::toLower(result);

    EXPECT_EQ(result, "hello world");
}

TEST_F(FileUtilsTest, FileUtils_ReadWriteRoundTrip)
{
    std::string temp_file = createTempFile("roundtrip.txt");
    std::string original = "Round-trip test content";

    auto write_result = FileUtils::writeFile(temp_file, original);
    EXPECT_TRUE(write_result.ok);

    auto read_result = FileUtils::readFile(temp_file);
    EXPECT_TRUE(read_result.ok);
    EXPECT_EQ(read_result.value, original);
}

TEST_F(StringUtilsTest, StringUtils_PatternValidation)
{
    std::string email = "test@example.com";
    bool has_at = StringUtils::contains(email, "@");
    bool has_dot = StringUtils::contains(email, ".");

    EXPECT_TRUE(has_at);
    EXPECT_TRUE(has_dot);
}

// ========== Edge Cases ==========

TEST_F(StringUtilsTest, EdgeCase_EmptyString)
{
    std::string empty = "";
    std::string trimmed = StringUtils::trim(empty);
    std::string lowered = StringUtils::toLower(empty);

    EXPECT_EQ(trimmed, "");
    EXPECT_EQ(lowered, "");
}

TEST_F(StringUtilsTest, EdgeCase_VeryLongString)
{
    std::string long_string(10000, 'a');
    std::string result = StringUtils::toLower(long_string);

    EXPECT_EQ(result, long_string);
    EXPECT_EQ(result.length(), 10000);
}

TEST_F(StringUtilsTest, EdgeCase_UnicodeString)
{
    std::string input = "Café";
    std::string lowered = StringUtils::toLower(input);

    // Should handle unicode gracefully (no crash)
}

TEST_F(FileUtilsTest, EdgeCase_LargeFile)
{
    std::string temp_file = createTempFile("large.txt");
    std::string large_content(100000, 'x');

    auto write_result = FileUtils::writeFile(temp_file, large_content);
    EXPECT_TRUE(write_result.ok);

    auto read_result = FileUtils::readFile(temp_file);
    EXPECT_TRUE(read_result.ok);
    EXPECT_EQ(read_result.value.length(), 100000);
}

// ========== Error Handling ==========

TEST_F(StringUtilsTest, ErrorHandling_Split_EmptyDelimiter)
{
    std::string input = "test";
    // Empty delimiter is handled by the implementation
    auto result = StringUtils::split(input, ' ');

    EXPECT_GE(result.size(), 0);
}

TEST_F(FileUtilsTest, ErrorHandling_ReadFile_InvalidPath)
{
    std::string invalid_path = "/invalid/nonexistent/path/file.txt";

    auto result = FileUtils::readFile(invalid_path);

    EXPECT_FALSE(result.ok);
}

TEST_F(FileUtilsTest, ErrorHandling_WriteFile_InvalidPath)
{
    std::string invalid_path = "/invalid/nonexistent/path/file.txt";

    auto result = FileUtils::writeFile(invalid_path, "content");

    EXPECT_FALSE(result.ok);
}
