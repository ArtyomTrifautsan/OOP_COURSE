#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <iostream>
#include <fstream>
#include <string>

#include <serialize.hpp>


template <typename T>
void test_simple_type(const T& value)
{
    std::string filename = "test.ser";
    std::ofstream ofs(filename, std::ofstream::out | std::ofstream::binary);
    serialize(value, ofs);

    ofs.close();

    std::ifstream ifs(filename, std::ifstream::in | std::ifstream::binary);
    ifs >> std::noskipws;

    T deserialized_value{};
    deserialize(deserialized_value, ifs);

    ifs.close();
    std::remove(filename.c_str());

    EXPECT_EQ(value, deserialized_value);
}


TEST(TestSimpleTypes, TestChar1) {
    test_simple_type<char>('A');
}

TEST(TestSimpleTypes, TestChar2) {
    test_simple_type<char>('\0');
}

TEST(TestSimpleTypes, TestChar3) {
    test_simple_type<char>('b');
}

TEST(TestSimpleTypes, TestSignedChar) {
    test_simple_type<signed char>(-127);
}

TEST(TestSimpleTypes, TestUnsignedChar) {
    test_simple_type<unsigned char>(255);
}


TEST(TestSimpleTypes, TestShort) {
    test_simple_type<short>(-32767);
}

TEST(TestSimpleTypes, TestUnsignedShort) {
    test_simple_type<unsigned short>(65535);
}

TEST(TestSimpleTypes, TestInt) {
    test_simple_type<int>(517);
}

TEST(TestSimpleTypes, TestUnsignedInt) {
    test_simple_type<unsigned int>(0xFFFFFFFFU);
}

TEST(TestSimpleTypes, TestLong) {
    test_simple_type<long>(-2147483647L);
}

TEST(TestSimpleTypes, TestUnsignedLong) {
    test_simple_type<unsigned long>(0xFFFFFFFFUL);
}

TEST(TestSimpleTypes, TestLongLong) {
    test_simple_type<long long>(-9223372036854775807LL);
}

TEST(TestSimpleTypes, TestUnsignedLongLong) {
    test_simple_type<unsigned long long>(0xFFFFFFFFFFFFFFFFULL);
}


TEST(TestSimpleTypes, TestFloat) {
    test_simple_type<float>(3.14159f);
}

TEST(TestSimpleTypes, TestDouble) {
    test_simple_type<double>(2.718281828459045);
}

TEST(TestSimpleTypes, TestLongDouble) {
    test_simple_type<long double>(3.141592653589793238L);
}


TEST(TestSimpleTypes, TestBoolTrue) {
    test_simple_type<bool>(true);
}

TEST(TestSimpleTypes, TestBoolFalse) {
    test_simple_type<bool>(false);
}