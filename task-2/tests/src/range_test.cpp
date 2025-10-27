#include <gtest/gtest.h>

#include <vector>

#include "filter_iterator.hpp"


TEST(TestFilterRange, VectorInt)
{
    struct MyPredicate
    {
        bool operator()(int a) { return a % 2 == 0; }
    };
    MyPredicate pred{};

    std::vector<int> v = {1, 2, 3, 4, 5, 6};

    Filter::Range range{pred, v.begin(), v.end()};

    std::vector<int> out{};
    for (auto item : range) out.push_back(item);

    EXPECT_EQ(out[0], 2);
    EXPECT_EQ(out[1], 4);
    EXPECT_EQ(out[2], 6);
}
