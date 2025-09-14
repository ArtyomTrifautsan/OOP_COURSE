#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <iostream>
#include <fstream>
#include <vector>

#include <serialize.hpp>


template <typename T>
void test_vector(const std::vector<T>& vector)
{
    std::string filename = "test.ser";
    std::ofstream ofs(filename, std::ofstream::out | std::ofstream::binary);
    serialize(vector, ofs);

    ofs.close();

    std::ifstream ifs(filename, std::ifstream::in | std::ifstream::binary);
    ifs >> std::noskipws;

    std::vector<T> deserialized_vector{};
    deserialize(deserialized_vector, ifs);

    ifs.close();
    std::remove(filename.c_str());

    EXPECT_EQ(vector, deserialized_vector);
}


TEST(TestVectors, VectorIntEmpty) {
    std::vector<int> vec{};
    test_vector(vec);
}

TEST(TestVectors, VectorIntSmall) {
    std::vector<int> vec = {1, 2, 3, 4, 5};
    test_vector(vec);
}

TEST(TestVectors, VectorIntWithZerosAndNegatives) {
    std::vector<int> vec = {-1, 0, 1, -100, 255, INT_MIN, INT_MAX};
    test_vector(vec);
}

TEST(TestVectors, VectorIntLarge) {
    std::vector<int> vec(1000);
    for (int i = 0; i < 1000; ++i) {
        vec[i] = i % 256;
    }
    test_vector(vec);
}

TEST(TestVectors, VectorDouble) {
    std::vector<double> vec = {0.0, 1.5, -2.718, 3.14159, 1e-10, 1e10};
    test_vector(vec);
}

TEST(TestVectors, VectorBool) {
    std::vector<bool> vec = {true, false, true, false, true};
    test_vector(vec);
}

TEST(TestVectors, VectorString) {
    std::vector<std::string> vec = {"", "hello", "world", "C++", "serialization test"};
    test_vector(vec);
}

TEST(TestVectors, VectorChar) {
    std::vector<char> vec = {'a', 'b', '\0', '\n', '\t', 'z'};
    test_vector(vec);
}

TEST(TestVectors, VectorLongLong) {
    std::vector<long long> vec = {LLONG_MIN, -1, 0, 1, LLONG_MAX};
    test_vector(vec);
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
    test_vector(vec);
}