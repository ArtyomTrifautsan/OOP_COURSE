#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <iostream>
#include <fstream>

#include <serialize_concepts.hpp>
#include "../my_vector.hpp"


template <typename T>
void test_my_vector(const MyVector<T>& vector)
{
    std::string filename = "test.ser";
    std::ofstream ofs(filename, std::ofstream::out | std::ofstream::binary);
    serialize(vector, ofs);

    ofs.close();

    std::ifstream ifs(filename, std::ifstream::in | std::ifstream::binary);
    ifs >> std::noskipws;

    MyVector<T> deserialized_vector{};
    deserialize(deserialized_vector, ifs);

    ifs.close();
    std::remove(filename.c_str());

    EXPECT_EQ(vector, deserialized_vector);
}


TEST(TestMyCustomVector, EmptyIntVector)
{
    MyVector<int> vector;
    test_my_vector(vector);
}

TEST(TestMyCustomVector, IntVectorWithElements)
{
    MyVector<int> vector;
    vector.push_back(42);
    vector.push_back(-7);
    vector.push_back(100);
    test_my_vector(vector);
}

TEST(TestMyCustomVector, StringVector)
{
    MyVector<std::string> vector;
    vector.push_back("hello");
    vector.push_back("world");
    vector.push_back("");
    test_my_vector(vector);
}

TEST(TestMyCustomVector, LargeIntVector)
{
    MyVector<int> vector;
    constexpr size_t N = 1000;
    for (int i = 0; i < N; ++i)
        vector.push_back(i * 2);
    test_my_vector(vector);
}

TEST(TestMyCustomVector, UserDefinedType)
{
    struct Point
    {
        int x, y;

        bool operator==(const Point& other) const noexcept
        {
            return x == other.x && y == other.y;
        }
    };

    MyVector<Point> vector;
    vector.push_back({1, 2});
    vector.push_back({3, 4});
    vector.push_back({5, 6});
    test_my_vector(vector);
}

TEST(TestMyCustomVector, ReuseAfterClear)
{
    MyVector<int> original;
    original.push_back(10);
    original.push_back(20);

    const std::string filename = "test.ser";
    {
        std::ofstream ofs(filename, std::ofstream::out | std::ofstream::binary);
        ASSERT_TRUE(ofs.is_open());
        serialize(original, ofs);
    }

    MyVector<int> restored;
    restored.push_back(999);    // Trash data
    restored.push_back(888);

    {
        std::ifstream ifs(filename, std::ifstream::in | std::ifstream::binary);
        ASSERT_TRUE(ifs.is_open());
        ifs >> std::noskipws;
        deserialize(restored, ifs);
    }

    std::remove(filename.c_str());

    EXPECT_EQ(restored.size(), 2u);
    EXPECT_EQ(restored[0], 10);
    EXPECT_EQ(restored[1], 20);
}
