#include <gtest/gtest.h>

#include <iostream>
#include <sstream>

#include <filter_iterator.hpp>


TEST(TestFilterIteratorView, TestEmptyInputIterator)
{
    struct MyPredicate
    {
        bool operator()(char a) { return a == 'a'; }
    };
    Filter::IteratorView<MyPredicate, std::istream_iterator<char>> filter_iterator{};

    EXPECT_EQ(filter_iterator.base(), filter_iterator.end());

    ++filter_iterator;
    EXPECT_EQ(filter_iterator.base(), filter_iterator.end());
}


TEST(TestFilterIteratorView, TestInputIterator)
{
    struct MyPredicate
    {
        bool operator()(char a) { return a == 'a'; }
    };
    std::istringstream iss("bca2ap");
    MyPredicate my_predicate{};
    std::istream_iterator<char> i_iter(iss);
    Filter::IteratorView filter_iterator{my_predicate, i_iter};

    EXPECT_NE(filter_iterator.base(), filter_iterator.end());

    std::ostringstream oss{};

    oss << *filter_iterator;
    EXPECT_EQ(oss.str(), "a");

    ++filter_iterator;
    EXPECT_NE(filter_iterator.base(), filter_iterator.end());

    oss << *filter_iterator;
    EXPECT_EQ(oss.str(), "aa");

    ++filter_iterator;
    EXPECT_EQ(filter_iterator.base(), filter_iterator.end());

    oss << *filter_iterator;
    EXPECT_EQ(oss.str(), "aap");

    ++filter_iterator;
    EXPECT_EQ(filter_iterator.base(), filter_iterator.end());

    oss << *filter_iterator;
    EXPECT_EQ(oss.str(), "aapp");

    ++filter_iterator;
    EXPECT_EQ(filter_iterator.base(), filter_iterator.end());
}


TEST(TestFilterIteratorView, TestInputIteratorWithDefaultPredicate)
{
    struct MyPredicate
    {
    public:
        MyPredicate() : comparing_value{'a'} {}
        MyPredicate(const char value) : comparing_value{value} {}
        bool operator()(char a) { return a != comparing_value; }
    private:
        char comparing_value{};
    };

    std::istringstream iss_1("abab n");

    // Default predicate
    std::istream_iterator<char> i_iter_1(iss_1);
    Filter::IteratorView<MyPredicate, std::istream_iterator<char>> filter_iterator{i_iter_1};

    EXPECT_NE(filter_iterator.base(), filter_iterator.end());

    std::ostringstream oss_1{};

    oss_1 << *filter_iterator;
    EXPECT_EQ(oss_1.str(), "b");

    ++filter_iterator;
    EXPECT_NE(filter_iterator.base(), filter_iterator.end());

    oss_1 << *filter_iterator;
    EXPECT_EQ(oss_1.str(), "bb");

    ++filter_iterator;
    EXPECT_NE(filter_iterator.base(), filter_iterator.end());

    oss_1 << *filter_iterator;
    EXPECT_EQ(oss_1.str(), "bbn");

    ++filter_iterator;
    EXPECT_EQ(filter_iterator.base(), filter_iterator.end());

    oss_1 << *filter_iterator;
    EXPECT_EQ(oss_1.str(), "bbnn");

    // Non-default predicate
    std::istringstream iss_2("abab n");
    MyPredicate my_predicate{'b'};
    std::istream_iterator<char> i_iter_2(iss_2);
    Filter::IteratorView filter_iterator_2{my_predicate, i_iter_2};

    EXPECT_NE(filter_iterator_2.base(), filter_iterator_2.end());

    std::ostringstream oss_2{};

    oss_2 << *filter_iterator_2;
    EXPECT_EQ(oss_2.str(), "a");

    ++filter_iterator_2;
    EXPECT_NE(filter_iterator_2.base(), filter_iterator_2.end());

    oss_2 << *filter_iterator_2;
    EXPECT_EQ(oss_2.str(), "aa");

    ++filter_iterator_2;
    EXPECT_NE(filter_iterator_2.base(), filter_iterator_2.end());

    oss_2 << *filter_iterator_2;
    EXPECT_EQ(oss_2.str(), "aan");

    ++filter_iterator_2;
    EXPECT_EQ(filter_iterator_2.base(), filter_iterator_2.end());

    oss_2 << *filter_iterator_2;
    EXPECT_EQ(oss_2.str(), "aann");
}