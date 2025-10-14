#include <gtest/gtest.h>

#include <vector>

#include <filter_iterator.hpp>


TEST(TestFilterIteratorRange, VectorInt)
{
    struct MyPredicate
    {
        bool operator()(int a) { return a % 2 == 0; }
    };

    std::vector<int> v = {1, 2, 3, 4, 5, 6};
    Filter::IteratorView iterator_begin{MyPredicate{}, v.begin(), v.end()};
    Filter::IteratorView iterator_end{MyPredicate{}, v.end(), v.end()};

    Filter::Range range{iterator_begin, iterator_end};

    std::vector<int> out{};
    for (auto item : range) out.push_back(item);

    EXPECT_EQ(out[0], 2);
    EXPECT_EQ(out[1], 4);
    EXPECT_EQ(out[2], 6);
}