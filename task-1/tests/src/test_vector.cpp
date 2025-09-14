#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <iostream>
#include <fstream>
#include <vector>
// #include <string>

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


TEST(TestContainers, VectorIntEmpty) {
    std::vector<int> vec{};
    test_vector(vec);
}

TEST(TestContainers, VectorIntSmall) {
    std::vector<int> vec = {1, 2, 3, 4, 5};
    test_vector(vec);
}

TEST(TestContainers, VectorIntWithZerosAndNegatives) {
    std::vector<int> vec = {-1, 0, 1, -100, 255, INT_MIN, INT_MAX};
    test_vector(vec);
}

TEST(TestContainers, VectorIntLarge) {
    std::vector<int> vec(1000);
    for (int i = 0; i < 1000; ++i) {
        vec[i] = i % 256;
    }
    test_vector(vec);
}

TEST(TestContainers, VectorDouble) {
    std::vector<double> vec = {0.0, 1.5, -2.718, 3.14159, 1e-10, 1e10};
    test_vector(vec);
}

TEST(TestContainers, VectorBool) {
    std::vector<bool> vec = {true, false, true, false, true};
    test_vector(vec);
}

TEST(TestContainers, VectorString) {
    std::vector<std::string> vec = {"", "hello", "world", "C++", "serialization test"};
    test_vector(vec);
}