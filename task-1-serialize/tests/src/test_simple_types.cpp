#include <gtest/gtest.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

// #include <serialize_concepts.hpp>
// #include <serialize_sfinae.hpp>
#include "serialize.hpp"


TEST(TestSimpleTypes, TestChar1)
{
    char value = 'A';

    std::ostringstream oss(std::stringstream::binary);

    serialize(value, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    char deserialized_value{};

    deserialize(deserialized_value, iss);

    EXPECT_EQ(value, deserialized_value);
}


TEST(TestSimpleTypes, TestChar2) 
{
    char value = '\0';

    std::ostringstream oss(std::stringstream::binary);

    serialize(value, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    char deserialized_value{};

    deserialize(deserialized_value, iss);

    EXPECT_EQ(value, deserialized_value);
}

TEST(TestSimpleTypes, TestChar3) 
{
    // char value = '你';
    char value = 'p';

    std::ostringstream oss(std::stringstream::binary);

    serialize(value, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    char deserialized_value{};

    deserialize(deserialized_value, iss);

    EXPECT_EQ(value, deserialized_value);
}

TEST(TestSimpleTypes, TestChar4) 
{
    // char value = '💡';
    char value = 'a';

    std::ostringstream oss(std::stringstream::binary);

    serialize(value, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    char deserialized_value{};

    deserialize(deserialized_value, iss);

    EXPECT_EQ(value, deserialized_value);
}

TEST(TestSimpleTypes, TestSignedChar) 
{
    signed char value = -127;

    std::ostringstream oss(std::stringstream::binary);

    serialize(value, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    signed char deserialized_value{};

    deserialize(deserialized_value, iss);

    EXPECT_EQ(value, deserialized_value);
}

TEST(TestSimpleTypes, TestUnsignedChar) 
{
    unsigned char value = 255;

    std::ostringstream oss(std::stringstream::binary);

    serialize(value, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    unsigned char deserialized_value{};

    deserialize(deserialized_value, iss);

    EXPECT_EQ(value, deserialized_value);
}


TEST(TestSimpleTypes, TestShort) 
{
    short value = -32767;

    std::ostringstream oss(std::stringstream::binary);

    serialize(value, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    short deserialized_value{};

    deserialize(deserialized_value, iss);

    EXPECT_EQ(value, deserialized_value);
}

TEST(TestSimpleTypes, TestUnsignedShort) 
{
    unsigned short value = 65535;

    std::ostringstream oss(std::stringstream::binary);

    serialize(value, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    unsigned short deserialized_value{};

    deserialize(deserialized_value, iss);

    EXPECT_EQ(value, deserialized_value);
}

TEST(TestSimpleTypes, TestInt) 
{
    int value = 517;

    std::ostringstream oss(std::stringstream::binary);

    serialize(value, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    int deserialized_value{};

    deserialize(deserialized_value, iss);

    EXPECT_EQ(value, deserialized_value);
}

TEST(TestSimpleTypes, TestUnsignedInt) 
{
    unsigned int value = 0xFFFFFFFFU;

    std::ostringstream oss(std::stringstream::binary);

    serialize(value, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    unsigned int deserialized_value{};

    deserialize(deserialized_value, iss);

    EXPECT_EQ(value, deserialized_value);
}

TEST(TestSimpleTypes, TestLong) 
{
    long value = -2147483647L;

    std::ostringstream oss(std::stringstream::binary);

    serialize(value, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    long deserialized_value{};

    deserialize(deserialized_value, iss);

    EXPECT_EQ(value, deserialized_value);
}

TEST(TestSimpleTypes, TestUnsignedLong) 
{
    unsigned long value = 0xFFFFFFFFUL;

    std::ostringstream oss(std::stringstream::binary);

    serialize(value, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    unsigned long deserialized_value{};

    deserialize(deserialized_value, iss);

    EXPECT_EQ(value, deserialized_value);
}

TEST(TestSimpleTypes, TestLongLong) 
{
    long long value = -9223372036854775807LL;

    std::ostringstream oss(std::stringstream::binary);

    serialize(value, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    long long deserialized_value{};

    deserialize(deserialized_value, iss);

    EXPECT_EQ(value, deserialized_value);
}

TEST(TestSimpleTypes, TestUnsignedLongLong) 
{
    unsigned long long value = 0xFFFFFFFFFFFFFFFFULL;

    std::ostringstream oss(std::stringstream::binary);

    serialize(value, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    unsigned long long deserialized_value{};

    deserialize(deserialized_value, iss);

    EXPECT_EQ(value, deserialized_value);
}


TEST(TestSimpleTypes, TestFloat) 
{
    float value = 3.14159f;

    std::ostringstream oss(std::stringstream::binary);

    serialize(value, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    float deserialized_value{};

    deserialize(deserialized_value, iss);

    EXPECT_EQ(value, deserialized_value);
}

TEST(TestSimpleTypes, TestDouble) 
{
    double value = 2.718281828459045;

    std::ostringstream oss(std::stringstream::binary);

    serialize(value, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    double deserialized_value{};

    deserialize(deserialized_value, iss);

    EXPECT_EQ(value, deserialized_value);
}

TEST(TestSimpleTypes, TestLongDouble) 
{
    long double value = 3.141592653589793238L;

    std::ostringstream oss(std::stringstream::binary);

    serialize(value, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    long double deserialized_value{};

    deserialize(deserialized_value, iss);

    EXPECT_EQ(value, deserialized_value);
}


TEST(TestSimpleTypes, TestBoolTrue) 
{
    bool value = true;

    std::ostringstream oss(std::stringstream::binary);

    serialize(value, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    bool deserialized_value{};

    deserialize(deserialized_value, iss);

    EXPECT_EQ(value, deserialized_value);
}

TEST(TestSimpleTypes, TestBoolFalse) {
    bool value = false;

    std::ostringstream oss(std::stringstream::binary);

    serialize(value, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    bool deserialized_value{};

    deserialize(deserialized_value, iss);

    EXPECT_EQ(value, deserialized_value);
}