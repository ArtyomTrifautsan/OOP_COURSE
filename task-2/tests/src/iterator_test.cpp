#include <gtest/gtest.h>

#include <iostream>
#include <sstream>
#include <vector>

#include "filter_iterator.hpp"


TEST(TestFilterIterator, TestDefaultConstructor)
{
    struct MyPredicate
    {
        MyPredicate() : m_basic_number{1} {}
        MyPredicate(int _number) : m_basic_number{_number} {}

        bool operator()(int _number) const noexcept { return _number == m_basic_number; }

        int basic_s() const noexcept { return m_basic_number; }

    private:
        int m_basic_number{};
    };

    Filter::Iterator<MyPredicate, std::vector<int>::iterator> filter_iter{};
    EXPECT_EQ(filter_iter.iter(), std::vector<int>::iterator{});
    EXPECT_EQ(filter_iter.end(), std::vector<int>::iterator{});

    MyPredicate pred = filter_iter.predicate();
    EXPECT_EQ(pred.basic_s(), 1);
}


TEST(TestFilterIterator, TestConstructorWithParameters)
{
    struct MyPredicate
    {
        bool operator()(int _number) const noexcept { return _number == 1; }
    };
    MyPredicate pred{};

    std::vector<int> v = {0, 1, 2, 3, 0, 1, 2, 3};

    Filter::Iterator filter_iter{pred, v.begin(), v.end()};

    EXPECT_NE(filter_iter.iter(), filter_iter.end());
    EXPECT_EQ(&(*filter_iter), v.data() + 1);
}


TEST(TestFilterIterator, TestCopyConstructor)
{
    struct MyPredicate;
    struct MyPredicate
    {
        bool operator()(int _number) const noexcept { return _number == 1; }
    };
    MyPredicate pred{};

    std::vector<int> v = {0, 1, 2, 3, 0, 1, 2, 3};

    Filter::Iterator filter_iter{pred, v.begin(), v.end()};
    Filter::Iterator copied_filter_iter{filter_iter};

    EXPECT_EQ(filter_iter.iter(), copied_filter_iter.iter());
    EXPECT_EQ(filter_iter.end(), copied_filter_iter.end());

    std::vector<int>::iterator remembered_iter = filter_iter.iter();
    ++filter_iter;

    EXPECT_NE(filter_iter.iter(), copied_filter_iter.iter());
    EXPECT_EQ(filter_iter.end(), copied_filter_iter.end());
    EXPECT_NE(remembered_iter, filter_iter.iter());
    EXPECT_EQ(remembered_iter, copied_filter_iter.iter());
}


TEST(TestFilterIterator, TestCopyAssignOperator)
{
    struct MyPredicate
    {
        bool operator()(int _number) const noexcept { return _number == 1; }
    };
    MyPredicate pred{};

    std::vector<int> v = {0, 1, 2, 3, 0, 1, 2, 3};

    Filter::Iterator filter_iter{pred, v.begin(), v.end()};

    std::vector<int> v2 = {0, 1, 2, 3};

    Filter::Iterator copied_filter_iter{pred, v.begin(), v.end()};
    EXPECT_NE(copied_filter_iter.iter(), copied_filter_iter.end());     // Просто чтоб компилятор не выкинул создание объекта copied_filter_iter

    copied_filter_iter = filter_iter;

    EXPECT_EQ(filter_iter.iter(), copied_filter_iter.iter());
    EXPECT_EQ(filter_iter.end(), copied_filter_iter.end());

    std::vector<int>::iterator remembered_iter = filter_iter.iter();
    ++filter_iter;

    EXPECT_NE(filter_iter.iter(), copied_filter_iter.iter());
    EXPECT_EQ(filter_iter.end(), copied_filter_iter.end());
    EXPECT_NE(remembered_iter, filter_iter.iter());
    EXPECT_EQ(remembered_iter, copied_filter_iter.iter());
}


TEST(TestFilterIterator, TestMoveConstructor)
{
    struct MyPredicate
    {
        bool operator()(int _number) const noexcept { return _number == 1; }
    };
    MyPredicate pred{};

    std::vector<int> v = {0, 1, 2, 3, 0, 1, 2, 3};

    Filter::Iterator filter_iter{pred, v.begin(), v.end()};

    std::vector<int>::iterator remembered_iter = filter_iter.iter();
    std::vector<int>::iterator remembered_end = filter_iter.end();

    Filter::Iterator copied_filter_iter{std::move(filter_iter)};

    EXPECT_EQ(copied_filter_iter.iter(), remembered_iter);
    EXPECT_EQ(copied_filter_iter.end(), remembered_end);
}


TEST(TestFilterIterator, TestMoveAssignOperator)
{
    struct MyPredicate
    {
        bool operator()(int _number) const noexcept { return _number == 1; }
    };
    MyPredicate pred{};

    std::vector<int> v = {0, 1, 2, 3, 0, 1, 2, 3};

    Filter::Iterator filter_iter{pred, v.begin(), v.end()};

    std::vector<int>::iterator remembered_iter = filter_iter.iter();
    std::vector<int>::iterator remembered_end = filter_iter.end();

    std::vector<int> v2 = {0, 1, 2, 3};

    Filter::Iterator copied_filter_iter{pred, v.begin(), v.end()};
    EXPECT_NE(copied_filter_iter.iter(), copied_filter_iter.end());     // Просто чтоб компилятор не выкинул создание объекта copied_filter_iter

    copied_filter_iter = std::move(filter_iter);

    EXPECT_EQ(copied_filter_iter.iter(), remembered_iter);
    EXPECT_EQ(copied_filter_iter.end(), remembered_end);
}



TEST(TestFilterIterator, TestDereferenceIteratorByPoint)
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
    Filter::Iterator filter_iter{pred, v.begin(), v.end()};

    EXPECT_EQ((*filter_iter).radius(), 2);

    ++filter_iter;

    EXPECT_EQ((*filter_iter).radius(), 4);
}


TEST(TestFilterIterator, TestDereferenceIteratorByArrow)
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
    Filter::Iterator filter_iter{pred, v.begin(), v.end()};

    EXPECT_EQ(filter_iter->radius(), 2);

    filter_iter++;

    EXPECT_EQ(filter_iter->radius(), 4);
}


TEST(TestFilterIterator, TestPrefixIncrement)
{
    struct MyPredicate
    {
        bool operator()(int _number) const noexcept { return _number == 1; }
    };
    MyPredicate pred{};

    std::vector<int> v = {0, 1, 2, 3, 0, 1, 2, 3};

    Filter::Iterator filter_iter{pred, v.begin(), v.end()};

    EXPECT_EQ(&(*filter_iter), v.data() + 1);

    ++filter_iter;

    EXPECT_EQ(&(*filter_iter), v.data() + 5);

    ++filter_iter;

    EXPECT_EQ(filter_iter.iter(), filter_iter.end());
}


TEST(TestFilterIterator, TestPostfixIncrement)
{
    struct MyPredicate
    {
        bool operator()(int _number) const noexcept { return _number == 1; }
    };
    MyPredicate pred{};

    std::vector<int> v = {0, 1, 2, 3, 0, 1, 2, 3};

    Filter::Iterator filter_iter{pred, v.begin(), v.end()};

    EXPECT_EQ(&(*filter_iter), v.data() + 1);

    auto copied_filter_iter = filter_iter++;

    EXPECT_EQ(&(*filter_iter), v.data() + 5);
    EXPECT_EQ(&(*copied_filter_iter), v.data() + 1);

    ++filter_iter;

    EXPECT_EQ(filter_iter.iter(), filter_iter.end());
    EXPECT_EQ(&(*copied_filter_iter), v.data() + 1);

    copied_filter_iter = filter_iter++;

    EXPECT_EQ(filter_iter.iter(), filter_iter.end());
    EXPECT_EQ(copied_filter_iter.iter(), copied_filter_iter.end());
}


TEST(TestFilterIterator, TestComparing)
{
    struct MyPredicate
    {
        bool operator()(char a) { return a == 'a'; }
    };
    MyPredicate my_predicate{};
    std::vector<char> v = {'a'};
    Filter::Iterator filter_iterator_begin{my_predicate, v.begin(), v.end()};
    Filter::Iterator filter_iterator_end{my_predicate, v.end(), v.end()};

    EXPECT_NE(filter_iterator_begin, filter_iterator_end);

    ++filter_iterator_begin;

    EXPECT_EQ(filter_iterator_begin, filter_iterator_end);
}


TEST(TestFilterIterator, TestEmptyVector)
{
    struct MyPredicate
    {
        bool operator()(char a) { return a == 'a'; }
    };
    std::vector<char> v = {};
    Filter::Iterator<MyPredicate, std::vector<char>::iterator> filter_iterator{};

    EXPECT_EQ(filter_iterator.iter(), filter_iterator.end());

    ++filter_iterator;

    EXPECT_EQ(filter_iterator.iter(), filter_iterator.end());
}