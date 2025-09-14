#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <iostream>
#include <fstream>
#include <string>

#include <serialize.hpp>


void test_string(const std::string& str)
{
    std::string filename = "test.ser";
    std::ofstream ofs(filename, std::ofstream::out | std::ofstream::binary);
    serialize(str, ofs);

    ofs.close();

    std::ifstream ifs(filename, std::ifstream::in | std::ifstream::binary);
    ifs >> std::noskipws;

    std::string deserialized_str{};
    deserialize(deserialized_str, ifs);

    ifs.close();
    std::remove(filename.c_str());

    EXPECT_EQ(str, deserialized_str);
}

TEST(TestString, StringEmpty) {
    std::string s = "";
    test_string(s);
}

TEST(TestString, StringSimple) {
    std::string s = "Hello, World!";
    test_string(s);
}

TEST(TestString, StringWithNullByte) {
    std::string s = "part1\0part2";
    test_string(s);
}

TEST(TestString, StringStartsWithNull) {
    std::string s = "\0abc";
    test_string(s);
}

TEST(TestString, StringEndsWithNull) {
    std::string s = "abc\0";
    test_string(s);
}

TEST(TestString, StringOnlyNull) {
    std::string s = "\0";
    test_string(s);
}

TEST(TestString, StringMultipleNulls) {
    std::string s = "\0\0\0\0";
    test_string(s);
}

TEST(TestString, StringWithNewlineTab) {
    std::string s = "Line1\nLine2\tTabbed";
    test_string(s);
}

TEST(TestString, StringWithCarriageReturn) {
    std::string s = "Windows\r\nline";
    test_string(s);
}

TEST(TestString, StringUnicodeUtf8) {
    std::string s = "Привет 🌍 🚀 💡 你好";
    test_string(s);
}

TEST(TestString, StringLong) {
    std::string s(10000, 'x');
    s[5000] = '\0';
    s[9999] = 'Z';
    test_string(s);
}

TEST(TestString, StringMaxSize) {
    std::string s(1'000'000, 'A');
    s[12345] = '\0';
    s[999999] = 'Z';
    test_string(s);
}