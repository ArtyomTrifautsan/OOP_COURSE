#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <iostream>
#include <fstream>
#include <vector>

// #include <serialize_concepts.hpp>
#include <serialize_sfinae.hpp>


// template <typename T>
// void test_vector(const std::vector<T>& vector)
// {
//     std::string filename = "test.ser";
//     std::ofstream ofs(filename, std::ofstream::out | std::ofstream::binary);
//     serialize(vector, ofs);

//     ofs.close();

//     std::ifstream ifs(filename, std::ifstream::in | std::ifstream::binary);
//     ifs >> std::noskipws;

//     std::vector<T> deserialized_vector{};
//     deserialize(deserialized_vector, ifs);

//     ifs.close();
//     std::remove(filename.c_str());

//     EXPECT_EQ(vector, deserialized_vector);
// }


TEST(TestVectors, VectorIntEmpty) {
    std::vector<int> vec{};
    std::ostringstream oss(std::stringstream::binary);

    serialize(vec, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    std::vector<int> deserialized_vector{};

    deserialize(deserialized_vector, iss);

    EXPECT_EQ(vec, deserialized_vector);

    vec.push_back(100);
    EXPECT_NE(vec, deserialized_vector);
}

TEST(TestVectors, VectorIntSmall) {
    std::vector<int> vec = {1, 2, 3, 4, 5};
    std::ostringstream oss(std::stringstream::binary);

    serialize(vec, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    std::vector<int> deserialized_vector{};

    deserialize(deserialized_vector, iss);

    EXPECT_EQ(vec, deserialized_vector);

    vec.push_back(100);
    EXPECT_NE(vec, deserialized_vector);
}

TEST(TestVectors, VectorIntWithZerosAndNegatives) {
    std::vector<int> vec = {-1, 0, 1, -100, 255, INT_MIN, INT_MAX};
    std::ostringstream oss(std::stringstream::binary);

    serialize(vec, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    std::vector<int> deserialized_vector{};

    deserialize(deserialized_vector, iss);

    EXPECT_EQ(vec, deserialized_vector);

    vec.push_back(100);
    EXPECT_NE(vec, deserialized_vector);
}

TEST(TestVectors, VectorIntLarge) {
    std::vector<int> vec(1000);
    for (int i = 0; i < 1000; ++i) {
        vec[i] = i % 256;
    }
    std::ostringstream oss(std::stringstream::binary);

    serialize(vec, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    std::vector<int> deserialized_vector{};

    deserialize(deserialized_vector, iss);

    EXPECT_EQ(vec, deserialized_vector);

    vec.push_back(100);
    EXPECT_NE(vec, deserialized_vector);
}

TEST(TestVectors, VectorDouble) {
    std::vector<double> vec = {0.0, 1.5, -2.718, 3.14159, 1e-10, 1e10};
    std::ostringstream oss(std::stringstream::binary);

    serialize(vec, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    std::vector<double> deserialized_vector{};

    deserialize(deserialized_vector, iss);

    EXPECT_EQ(vec, deserialized_vector);

    vec.push_back(100.0);
    EXPECT_NE(vec, deserialized_vector);
}

TEST(TestVectors, VectorBool) {
    std::vector<bool> vec = {true, false, true, false, true};
    std::ostringstream oss(std::stringstream::binary);

    serialize(vec, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    std::vector<bool> deserialized_vector{};

    deserialize(deserialized_vector, iss);

    EXPECT_EQ(vec, deserialized_vector);

    vec.push_back(false);
    EXPECT_NE(vec, deserialized_vector);
}

TEST(TestVectors, VectorString) {
    std::vector<std::string> vec = {"", "hello", "world", "C++", "serialization test"};
    std::ostringstream oss(std::stringstream::binary);

    serialize(vec, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    std::vector<std::string> deserialized_vector{};

    deserialize(deserialized_vector, iss);

    EXPECT_EQ(vec, deserialized_vector);

    vec.push_back("100");
    EXPECT_NE(vec, deserialized_vector);
}

TEST(TestVectors, VectorChar) {
    std::vector<char> vec = {'a', 'b', '\0', '\n', '\t', 'z'};
    std::ostringstream oss(std::stringstream::binary);

    serialize(vec, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    std::vector<char> deserialized_vector{};

    deserialize(deserialized_vector, iss);

    EXPECT_EQ(vec, deserialized_vector);

    vec.push_back(100);
    EXPECT_NE(vec, deserialized_vector);
}

TEST(TestVectors, VectorLongLong) {
    std::vector<long long> vec = {LLONG_MIN, -1, 0, 1, LLONG_MAX};
    std::ostringstream oss(std::stringstream::binary);

    serialize(vec, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    std::vector<long long> deserialized_vector{};

    deserialize(deserialized_vector, iss);

    EXPECT_EQ(vec, deserialized_vector);

    vec.push_back(100);
    EXPECT_NE(vec, deserialized_vector);
}

TEST(TestVectors, VectorVectorVectorString) {
    std::vector<std::vector<std::vector<std::string>>> vec = {
        {
            {"1", "2", "3"},
            {"a", "b", "c", "d", "e"},
            {"one", "two"}
        },
        {
            {"qwerty", "=qwerty="},
            {},
            {"my", "string", "best"},
            {"Vasily Perdenko"},
            {"frist", "second", "third", "", "fifth", "sixth"}
        },
        {

        }
    };
    std::ostringstream oss(std::stringstream::binary);

    serialize(vec, oss);

    std::istringstream iss(oss.str(), std::stringstream::binary);
    iss >> std::noskipws;

    std::vector<std::vector<std::vector<std::string>>> deserialized_vector{};

    deserialize(deserialized_vector, iss);

    EXPECT_EQ(vec, deserialized_vector);
}