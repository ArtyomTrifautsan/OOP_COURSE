#include <gtest/gtest.h>

#include <iostream>
#include <fstream>
#include <string>

// #include <serialize_concepts.hpp>
// #include <serialize_sfinae.hpp>
#include "serialize.hpp"


TEST(TestString, StringEmpty) 
{
    std::string str = "";

    std::ostringstream oss(std::stringstream::binary);

    serialize(str, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    std::string deserialized_str{};

    deserialize(deserialized_str, iss);

    EXPECT_EQ(str, deserialized_str);
}

TEST(TestString, StringSimple) 
{
    std::string str = "Hello, World!";

    std::ostringstream oss(std::stringstream::binary);

    serialize(str, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    std::string deserialized_str{};

    deserialize(deserialized_str, iss);

    EXPECT_EQ(str, deserialized_str);
}

TEST(TestString, StringWithNullByte) {
    std::string s = "part1\0part2";

    std::ostringstream oss(std::stringstream::binary);

    serialize(s, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    std::string deserialized_str{};

    deserialize(deserialized_str, iss);

    EXPECT_EQ(s, deserialized_str);
}

TEST(TestString, StringStartsWithNull) {
    std::string s = "\0abc";
    std::ostringstream oss(std::stringstream::binary);

    serialize(s, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    std::string deserialized_str{};

    deserialize(deserialized_str, iss);

    EXPECT_EQ(s, deserialized_str);
}

TEST(TestString, StringEndsWithNull) {
    std::string s = "abc\0";
    std::ostringstream oss(std::stringstream::binary);

    serialize(s, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    std::string deserialized_str{};

    deserialize(deserialized_str, iss);

    EXPECT_EQ(s, deserialized_str);
}

TEST(TestString, StringOnlyNull) {
    std::string s = "\0";
    std::ostringstream oss(std::stringstream::binary);

    serialize(s, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    std::string deserialized_str{};

    deserialize(deserialized_str, iss);

    EXPECT_EQ(s, deserialized_str);
}

TEST(TestString, StringMultipleNulls) {
    std::string s = "\0\0\0\0";
    std::ostringstream oss(std::stringstream::binary);

    serialize(s, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    std::string deserialized_str{};

    deserialize(deserialized_str, iss);

    EXPECT_EQ(s, deserialized_str);
}

TEST(TestString, StringWithNewlineTab) {
    std::string s = "Line1\nLine2\tTabbed";
    std::ostringstream oss(std::stringstream::binary);

    serialize(s, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    std::string deserialized_str{};

    deserialize(deserialized_str, iss);

    EXPECT_EQ(s, deserialized_str);
}

TEST(TestString, StringWithCarriageReturn) {
    std::string s = "Windows\r\nline";
    std::ostringstream oss(std::stringstream::binary);

    serialize(s, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    std::string deserialized_str{};

    deserialize(deserialized_str, iss);

    EXPECT_EQ(s, deserialized_str);
}

TEST(TestString, StringUnicodeUtf8) {
    std::string s = "Привет 🌍 🚀 💡 你好";
    std::ostringstream oss(std::stringstream::binary);

    serialize(s, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    std::string deserialized_str{};

    deserialize(deserialized_str, iss);

    EXPECT_EQ(s, deserialized_str);
}

TEST(TestString, StringLong) {
    std::string s(10000, 'x');
    s[5000] = '\0';
    s[9999] = 'Z';
    std::ostringstream oss(std::stringstream::binary);

    serialize(s, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    std::string deserialized_str{};

    deserialize(deserialized_str, iss);

    EXPECT_EQ(s, deserialized_str);
}

TEST(TestString, StringMaxSize) {
    std::string s(1'000'000, 'A');
    s[12345] = '\0';
    s[999999] = 'Z';
    std::ostringstream oss(std::stringstream::binary);

    serialize(s, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    std::string deserialized_str{};

    deserialize(deserialized_str, iss);

    EXPECT_EQ(s, deserialized_str);
}