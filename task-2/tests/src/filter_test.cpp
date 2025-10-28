#include <gtest/gtest.h>

#include <vector>
#include <algorithm>

#include "filter_iterator.hpp"


/*
Добавить лямбда функции, простые функции 
*/

namespace {
    struct MyPredicate
    {
        bool operator()(int a) { return a % 2 == 0; }
    };
}
TEST(TestFilterRange, VectorInt)
{
    MyPredicate pred{};

    std::vector<int> v = {1, 2, 3, 4, 5, 6};

    Filter::Range range{pred, v.begin(), v.end()};

    std::vector<int> out{};
    for (auto item : range) out.push_back(item);

    EXPECT_EQ(out[0], 2);
    EXPECT_EQ(out[1], 4);
    EXPECT_EQ(out[2], 6);
}


TEST(TestFilterIterator, IteratorConstructorWithParameters1)
{
    struct MyPredicate
    {
        bool operator()(int _number) const noexcept { return _number == 1; }
    };
    MyPredicate pred{};

    std::vector<int> v = {};

    Filter::Range range{MyPredicate{}, v.begin(), v.end()};

    auto filter_iter_begin = range.begin();
    auto filter_iter_end = range.end();

    EXPECT_EQ(filter_iter_begin, filter_iter_end);
}


TEST(TestFilterIterator, IteratorConstructorWithParameters2)
{
    struct MyPredicate
    {
        bool operator()(int _number) const noexcept { return _number == 1; }
    };
    MyPredicate pred{};

    std::vector<int> v = {0, 1, 2, 3, 0, 1, 2, 3};

    Filter::Range range{pred, v.begin(), v.end()};

    auto filter_iter_begin = range.begin();
    auto filter_iter_end = range.end();

    EXPECT_NE(filter_iter_begin, filter_iter_end);

    ++filter_iter_begin;

    EXPECT_NE(filter_iter_begin, filter_iter_end);

    ++filter_iter_begin;

    EXPECT_EQ(filter_iter_begin, filter_iter_end);
}


TEST(TestFilterIterator, IteratorCopyConstructor)
{
    // struct MyPredicate;
    struct MyPredicate
    {
        bool operator()(int _number) const noexcept { return _number == 1; }
    };
    MyPredicate pred{};

    std::vector<int> v = {0, 1, 2, 3, 0, 1, 2, 3};

    Filter::Range range{pred, v.data(), v.data() + v.size()};

    auto iter_begin = range.begin();
    auto iter_end = range.end();

    auto iter_begin_copy = iter_begin;

    EXPECT_EQ(iter_begin, iter_begin_copy);

    ++iter_begin;

    EXPECT_NE(iter_begin, iter_begin_copy);

    ++iter_begin_copy;

    EXPECT_EQ(iter_begin, iter_begin_copy);

    ++iter_begin_copy;

    EXPECT_EQ(iter_begin_copy, iter_end);
}


TEST(TestFilterIterator, IteratorCopyAssignOperator)
{
    struct MyPredicate
    {
        bool operator()(int _number) const noexcept { return _number == 1; }
    };
    MyPredicate pred{};

    std::vector<int> v = {0, 1, 2, 3, 0, 1, 2, 3};

    Filter::Range range{pred, v.begin(), v.end()};

    auto iter_begin = range.begin();
    auto iter_end = range.end();

    // Просто чтоб компилятор не выкинул создание объекта copied_filter_iter
    //std::vector<int> v2 = {0, 1, 2, 3};
    Filter::Range range2{pred, v.begin(), v.end()};
    auto iter_begin_2 = range2.begin();
    EXPECT_NE(iter_begin_2, range2.end());     

    iter_begin_2 = iter_begin;

    EXPECT_EQ(iter_begin, iter_begin_2);

    ++iter_begin;

    EXPECT_NE(iter_begin, iter_begin_2);

    ++iter_begin_2;

    EXPECT_EQ(iter_begin, iter_begin_2);

    ++iter_begin_2;

    EXPECT_EQ(iter_begin_2, iter_end);
}


TEST(TestFilterIterator, DereferenceIteratorByPoint)
{
    struct Circle
    {
        Circle() : m_radius{-1} {}
        Circle(int _radius) : m_radius{_radius} {}
        int radius() const { return m_radius; }
        int m_radius{};
    };

    struct MyPredicate
    {
        bool operator()(const Circle& c) const noexcept { return c.radius() % 2 == 0; }
    };
    MyPredicate pred{};

    std::vector<Circle> v{};
    for (int i = 1; i < 5; i++) v.push_back(Circle(i));
    Filter::Range range{pred, v.begin(), v.end()};

    auto iter_begin = range.begin();
    auto iter_end = range.end();

    EXPECT_EQ((*iter_begin).radius(), 2);

    ++iter_begin;

    EXPECT_EQ((*iter_begin).radius(), 4);
}


TEST(TestFilterIterator, DereferenceIteratorByArrow)
{
    struct Circle
    {
        Circle() : m_radius{-1} {}
        Circle(int _radius) : m_radius{_radius} {}
        int radius() const { return m_radius; }
        int m_radius{};
    };

    struct MyPredicate
    {
        bool operator()(const Circle& c) const noexcept { return c.radius() % 2 == 0; }
    };
    MyPredicate pred{};

    std::vector<Circle> v{};
    for (int i = 1; i < 5; i++) v.push_back(Circle(i));
    Filter::Range range{pred, v.begin(), v.end()};

    auto iter_begin = range.begin();
    auto iter_end = range.end();

    EXPECT_EQ(iter_begin->radius(), 2);

    ++iter_begin;

    EXPECT_EQ(iter_begin->radius(), 4);
}


TEST(TestFilterIterator, PrefixIncrement)
{
    struct MyPredicate
    {
        bool operator()(int _number) const noexcept { return _number == 1; }
    };
    MyPredicate pred{};

    std::vector<int> v = {0, 1, 2, 3, 0, 1, 2, 3};

    Filter::Range range{pred, v.begin(), v.end()};

    auto iter_begin = range.begin();
    auto iter_end = range.end();

    EXPECT_EQ(&(*iter_begin), v.data() + 1);

    ++iter_begin;

    EXPECT_EQ(&(*iter_begin), v.data() + 5);

    ++iter_begin;

    EXPECT_EQ(iter_begin, iter_end);
}


TEST(TestFilterIterator, PostfixIncrement)
{
    struct MyPredicate
    {
        bool operator()(int _number) const noexcept { return _number == 1; }
    };
    MyPredicate pred{};

    std::vector<int> v = {0, 1, 2, 3, 0, 1, 2, 3};

    Filter::Range range{pred, v.begin(), v.end()};

    auto iter_begin = range.begin();
    auto iter_end = range.end();

    EXPECT_EQ(&(*iter_begin), v.data() + 1);

    auto iter_begin_copy = iter_begin++;

    EXPECT_NE(iter_begin, iter_begin_copy);
    EXPECT_EQ(&(*iter_begin), v.data() + 5);
    EXPECT_EQ(&(*iter_begin_copy), v.data() + 1);

    ++iter_begin_copy;

    EXPECT_EQ(iter_begin, iter_begin_copy);

    iter_begin_copy = iter_begin++;

    EXPECT_EQ(iter_begin, iter_end);
    EXPECT_EQ(&(*iter_begin_copy), v.data() + 5);

    ++iter_begin_copy;

    EXPECT_EQ(iter_begin, iter_begin_copy);
}


TEST(TestFilterIterator, Comparing)
{
    struct MyPredicate
    {
        bool operator()(char a) { return a == 'a'; }
    };
    MyPredicate pred{};
    std::vector<char> v = {'a'};

    Filter::Range range{pred, v.begin(), v.end()};

    auto iter_begin = range.begin();
    auto iter_end = range.end();

    EXPECT_NE(iter_begin, iter_end);

    ++iter_begin;

    EXPECT_EQ(iter_begin, iter_end);
}


TEST(TestFilterIteratorWithSTL, Distance)
{
    struct IsEven {
        bool operator()(int x) const { return x % 2 == 0; }
    };
    IsEven pred{};
    std::vector<int> v = {1, 2, 3, 4, 5, 6};

    Filter::Range range{pred, v.begin(), v.end()};

    EXPECT_EQ(std::distance(range.begin(), range.end()), 3);
}


TEST(TestFilterIteratorWithSTL, Find) {
    struct IsEven {
        bool operator()(int x) const { return x % 2 == 0; }
    };
    std::vector<int> v = {1, 2, 3, 4, 5, 6};
    Filter::Range range{IsEven{}, v.begin(), v.end()};
    
    auto it = std::find(range.begin(), range.end(), 4);
    EXPECT_NE(it, range.end());
    EXPECT_EQ(*it, 4);

    it = std::find(range.begin(), range.end(), 3);
    EXPECT_EQ(it, range.end());
}


TEST(TestFilterIteratorWithSTL, Copy) {
    struct IsEven {
        bool operator()(int x) const { return x % 2 == 0; }
    };
    std::vector<int> v = {1, 2, 3, 4, 5, 6};
    std::vector<int> result{};
    
    Filter::Range range{IsEven{}, v.begin(), v.end()};

    std::copy(range.begin(), range.end(), std::back_inserter(result));

    EXPECT_EQ(result, std::vector<int>({2, 4, 6}));
}


TEST(TestFilterIteratorWithSTL, ForEach) {
    struct IsEven {
        bool operator()(int x) const { return x % 2 == 0; }
    };

    std::vector<int> v = {1, 2, 3, 4, 5, 6};

    Filter::Range range{IsEven{}, v.begin(), v.end()};
    
    int sum = 0;
    std::for_each(range.begin(), range.end(), [&sum](int x) { sum += x; });
    EXPECT_EQ(sum, 12);
}


TEST(TestFilterIteratorWithSTL, MaxElement) {
    struct IsEven {
        bool operator()(int x) const { return x % 2 == 0; }
    };

    std::vector<int> v = {1, 5, 3, 8, 2};

    Filter::Range range{IsEven{}, v.begin(), v.end()};

    auto it = std::max_element(range.begin(), range.end());
    EXPECT_EQ(*it, 8);
}
