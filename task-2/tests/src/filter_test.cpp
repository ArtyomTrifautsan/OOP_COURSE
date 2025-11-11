#include <gtest/gtest.h>

#include <vector>
#include <list>
#include <forward_list>
#include <map>
#include <algorithm>

#include "filter_iterator.hpp"


/*
Добавить лямбда функции, простые функции 
*/

namespace {
    struct IsEven
    {
        bool operator()(int a) { return a % 2 == 0; }
    };

    struct Circle
    {
        Circle() : m_radius{-1} {}
        Circle(int _radius) : m_radius{_radius} {}
        int radius() const { return m_radius; }
        int m_radius{};
    };

    struct IsCircleRadiusEven
    {
        bool operator()(const Circle& c) const noexcept { return c.radius() % 2 == 0; }
    };

    struct IsKeyEven {
        bool operator()(const std::pair<const int, std::string>& p) const 
        {
            return p.first % 2 == 0;
        }
    };

    bool is_even_func(int x) { return x % 2 == 0; }
}


TEST(Constructor, Copy)
{
    std::vector<int> v = {1, 2, 3, 4, 5, 6, 7};

    Filter::Range range{IsEven{}, v.begin(), v.end()};

    auto iter_begin = range.begin();

    auto iter_begin_copy = iter_begin;

    EXPECT_EQ(iter_begin, iter_begin_copy);

    ++iter_begin;

    EXPECT_NE(iter_begin, iter_begin_copy);

    ++iter_begin_copy;

    EXPECT_EQ(iter_begin, iter_begin_copy);
}


TEST(AssignOperator, Copy)
{
    std::vector<int> v = {1, 2, 3, 4, 5, 6, 7};

    Filter::Range range{IsEven{}, v.begin(), v.end()};

    auto iter_begin = range.begin();

    Filter::Range range2{IsEven{}, v.begin(), v.end()};

    auto iter_begin_2 = range2.begin();

    EXPECT_NE(iter_begin_2, range2.end());

    iter_begin_2 = iter_begin;

    EXPECT_EQ(iter_begin, iter_begin_2);

    ++iter_begin;

    EXPECT_NE(iter_begin, iter_begin_2);

    ++iter_begin_2;

    EXPECT_EQ(iter_begin, iter_begin_2);
}


TEST(Dereference, ByAsterisk)
{
    std::vector<int> v = {1, 2, 3, 4, 5, 6, 7};

    Filter::Range range{IsEven{}, v.begin(), v.end()};

    auto iter_begin = range.begin();

    EXPECT_EQ(*iter_begin, 2);

    ++iter_begin;

    EXPECT_EQ(*iter_begin, 4);

    ++iter_begin;

    EXPECT_EQ(*iter_begin, 6);
}


TEST(Dereference, ByArrow)
{
    std::vector<Circle> v{};
    for (int i = 1; i < 8; i++) v.push_back(Circle(i));
    Filter::Range range{IsCircleRadiusEven{}, v.begin(), v.end()};

    auto iter_begin = range.begin();

    EXPECT_EQ(iter_begin->radius(), 2);

    ++iter_begin;

    EXPECT_EQ(iter_begin->radius(), 4);

    ++iter_begin;

    EXPECT_EQ(iter_begin->radius(), 6);
}


TEST(PrefixIncrement, MovementAlongContainer)
{
    std::vector<int> v = {1, 2, 3, 4, 5, 6, 7};

    Filter::Range range{IsEven{}, v.begin(), v.end()};

    auto iter_begin = range.begin();
    auto iter_end = range.end();

    EXPECT_EQ(*iter_begin, v[1]);

    ++iter_begin;

    EXPECT_EQ(*iter_begin, v[3]);

    ++iter_begin;

    EXPECT_EQ(*iter_begin, v[5]);

    ++iter_begin;

    EXPECT_EQ(iter_begin, iter_end);
}


TEST(PrefixIncrement, SelfReturn)
{
    std::vector<int> v = {1, 2, 3, 4, 5, 6, 7};

    Filter::Range range{IsEven{}, v.begin(), v.end()};

    auto iter_begin = range.begin();
    auto iter_end = range.end();

    auto iter_copy = ++iter_begin;

    EXPECT_EQ(iter_begin, iter_copy);
}


TEST(PostfixIncrement, MovementAlongContainer)
{
    std::vector<int> v = {1, 2, 3, 4, 5, 6, 7};

    Filter::Range range{IsEven{}, v.begin(), v.end()};

    auto iter_begin = range.begin();
    auto iter_end = range.end();

    EXPECT_EQ(*iter_begin, v[1]);

    iter_begin++;

    EXPECT_EQ(*iter_begin, v[3]);

    iter_begin++;

    EXPECT_EQ(*iter_begin, v[5]);

    iter_begin++;

    EXPECT_EQ(iter_begin, iter_end);
}


TEST(PostfixIncrement, CopyReturn)
{
    std::vector<int> v = {1, 2, 3, 4, 5, 6, 7};

    Filter::Range range{IsEven{}, v.begin(), v.end()};

    auto iter_begin = range.begin();
    auto iter_end = range.end();

    auto iter_copy = iter_begin++;

    EXPECT_NE(iter_begin, iter_copy);

    iter_copy++;

    EXPECT_EQ(iter_begin, iter_copy);
}


TEST(TestFilterIterator, Comparing)
{
    std::vector<int> v = {1, 2, 3, 4, 5, 6, 7};

    Filter::Range range{IsEven{}, v.begin(), v.end()};

    auto iter_begin = range.begin();
    auto iter_end = range.end();
    auto iter_copy = iter_begin;

    EXPECT_NE(iter_begin, iter_end);
}


TEST(STL, Distance)
{
    struct IsEven {
        bool operator()(int x) const { return x % 2 == 0; }
    };
    IsEven pred{};
    std::vector<int> v = {1, 2, 3, 4, 5, 6};

    Filter::Range range{pred, v.begin(), v.end()};

    EXPECT_EQ(std::distance(range.begin(), range.end()), 3);
}


TEST(STL, Find) {
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


TEST(STL, Copy) {
    struct IsEven {
        bool operator()(int x) const { return x % 2 == 0; }
    };
    std::vector<int> v = {1, 2, 3, 4, 5, 6};
    std::vector<int> result{};
    
    Filter::Range range{IsEven{}, v.begin(), v.end()};

    std::copy(range.begin(), range.end(), std::back_inserter(result));

    EXPECT_EQ(result, std::vector<int>({2, 4, 6}));
}


TEST(STL, ForEach) {
    struct IsEven {
        bool operator()(int x) const { return x % 2 == 0; }
    };

    std::vector<int> v = {1, 2, 3, 4, 5, 6};

    Filter::Range range{IsEven{}, v.begin(), v.end()};
    
    int sum = 0;
    std::for_each(range.begin(), range.end(), [&sum](int x) { sum += x; });
    EXPECT_EQ(sum, 12);
}


TEST(STL, MaxElement) {
    struct IsEven {
        bool operator()(int x) const { return x % 2 == 0; }
    };

    std::vector<int> v = {1, 5, 3, 8, 2};

    Filter::Range range{IsEven{}, v.begin(), v.end()};

    auto it = std::max_element(range.begin(), range.end());
    EXPECT_EQ(*it, 8);
}


TEST(Vector, Empty)
{
    std::vector<int> v = {};

    Filter::Range range{IsEven{}, v.begin(), v.end()};

    auto filter_iter_begin = range.begin();
    auto filter_iter_end = range.end();

    EXPECT_EQ(filter_iter_begin, filter_iter_end);
}


TEST(Vector, OfInt)
{
    std::vector<int> v = {1, 2, 3, 4, 5, 6};
    Filter::Range range{IsEven{}, v.begin(), v.end()};

    std::vector<int> result(range.begin(), range.end());
    EXPECT_EQ(result, std::vector<int>({2, 4, 6}));
}


TEST(List, OfInt)
{
    std::list<int> l = {1, 2, 3, 4, 5, 6};
    Filter::Range range{IsEven{}, l.begin(), l.end()};

    std::vector<int> result(range.begin(), range.end());
    EXPECT_EQ(result, std::vector<int>({2, 4, 6}));
}


TEST(ForwardList, OfInt)
{
    std::forward_list<int> fl = {2, 3, 4, 5, 6, 7, 8};
    Filter::Range range{IsEven{}, fl.begin(), fl.end()};

    std::vector<int> result;
    std::copy(range.begin(), range.end(), std::back_inserter(result));
    EXPECT_EQ(result, std::vector<int>({2, 4, 6, 8}));
}


TEST(FilterIterator, Map)
{
    std::map<int, std::string> m = {
        {1, "one"},
        {2, "two"},
        {3, "three"},
        {4, "four"}
    };

    Filter::Range range{IsKeyEven{}, m.begin(), m.end()};

    std::vector<std::pair<int, std::string>> result(range.begin(), range.end());
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].first, 2);
    EXPECT_EQ(result[0].second, "two");
    EXPECT_EQ(result[1].first, 4);
    EXPECT_EQ(result[1].second, "four");
}


TEST(RangeBasedLoop, RangeBasedLoop)
{
    std::vector<int> v = {1, 2, 3, 4, 5, 6, 7};

    Filter::Range range{IsEven{}, v.begin(), v.end()};

    std::vector<int> out{};
    for (auto item : range) out.push_back(item);

    EXPECT_EQ(out[0], 2);
    EXPECT_EQ(out[1], 4);
    EXPECT_EQ(out[2], 6);
}


TEST(PointerToFunction, FunctionPointer)
{
    std::vector<int> v = {1, 2, 3, 4, 5, 6, 7, 8};

    Filter::Range range{&is_even_func, v.begin(), v.end()};

    std::vector<int> result(range.begin(), range.end());
    EXPECT_EQ(result, std::vector<int>({2, 4, 6, 8}));
}


TEST(Lambda, NoCapture) {
    std::vector<int> v = {10, 15, 20, 25};
    auto is_multiple_of_5 = [](int x) { return x % 5 == 0; };

    Filter::Range range{is_multiple_of_5, v.begin(), v.end()};
    std::vector<int> result(range.begin(), range.end());
    EXPECT_EQ(result, v);
}


TEST(Lambda, WithCapture) {
    std::vector<int> v = {1, 2, 3, 4, 5, 6};
    int threshold = 3;

    // Лямбда с захватом — тип уникален и не-копируемый? Нет, захваченные значения копируются.
    auto greater_than = [threshold](int x) { return x > threshold; };

    Filter::Range range{greater_than, v.begin(), v.end()};
    std::vector<int> result(range.begin(), range.end());
    EXPECT_EQ(result, std::vector<int>({4, 5, 6}));
}


TEST(StdFunction, StdFunction) {
    std::vector<int> v = {-2, -1, 0, 1, 2};
    std::function<bool(int)> pred = [](int x) { return x >= 0; };

    Filter::Range range{pred, v.begin(), v.end()};
    std::vector<int> result(range.begin(), range.end());
    EXPECT_EQ(result, std::vector<int>({0, 1, 2}));
}



// TEST(TestFilterIterator, IteratorConstructorWithParameters2)
// {
//     struct MyPredicate
//     {
//         bool operator()(int _number) const noexcept { return _number == 1; }
//     };
//     MyPredicate pred{};

//     std::vector<int> v = {0, 1, 2, 3, 0, 1, 2, 3};

//     Filter::Range range{pred, v.begin(), v.end()};

//     auto filter_iter_begin = range.begin();
//     auto filter_iter_end = range.end();

//     EXPECT_NE(filter_iter_begin, filter_iter_end);

//     ++filter_iter_begin;

//     EXPECT_NE(filter_iter_begin, filter_iter_end);

//     ++filter_iter_begin;

//     EXPECT_EQ(filter_iter_begin, filter_iter_end);
// }


// TEST(Dereference, ByAsterisk)
// {
//     std::vector<Circle> v{};
//     for (int i = 1; i < 8; i++) v.push_back(Circle(i));
//     Filter::Range range{IsCircleRadiusEven{}, v.begin(), v.end()};

//     auto iter_begin = range.begin();
//     auto iter_end = range.end();

//     EXPECT_EQ((*iter_begin).radius(), 2);

//     ++iter_begin;

//     EXPECT_EQ((*iter_begin).radius(), 4);
// }