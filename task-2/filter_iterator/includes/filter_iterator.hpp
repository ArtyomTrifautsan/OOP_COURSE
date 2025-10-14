#pragma once

#include <iterator>
#include <type_traits>

/*
Что не так:
1) называть архивы с исходниками одинаково
2) Убрать дефолтное значение Iterator end = Iterator()
3) Убрать конструктор без предиката в IteratorView
4) Переименовать IteratorView в Iterator
5) Проверять все методы в IteratorView
6) Проверить операторы и конструкторы копирования и присваивания в IteratorView и возможно создать если нужно
7) Добавить разыминовывание через стрелочку, префиксный и постфиксный ++ (то есть все что нужно для ForwardIterator и LegacyIterator)
*/


namespace Filter {

    template <typename Predicate, typename Iterator>
    class IteratorView
    {
    public:
        using value_type = std::iterator_traits<Iterator>::value_type;
        using reference = std::iterator_traits<Iterator>::reference;
        using pointer = std::iterator_traits<Iterator>::pointer;
        using difference_type = std::iterator_traits<Iterator>::difference_type;

        //  Пока так, а вообще тут должен быть либо forward_iterator, либо input_iterator
        using iterator_category = std::forward_iterator_tag;


        IteratorView() = default;

        IteratorView(Predicate f, Iterator x, Iterator end = Iterator()) : m_predicate{f}, m_iter{x}, m_end{end}
        {
            go_to_closest_valid_item();
        }

        // Убрать
        IteratorView(Iterator x, Iterator end = Iterator()) : m_predicate{}, m_iter{x}, m_end{end} 
        {
            go_to_closest_valid_item();
        }

        // убрать эти 3 метода
        Predicate predicate() const { return m_predicate; }
        Iterator end() const { return m_end; }
        Iterator const& base() const { return m_iter; }

        reference operator*() const { return *m_iter; }

        IteratorView& operator++()
        {
            if (m_iter != m_end) ++m_iter;
            go_to_closest_valid_item();
            return *this;
        }

        friend bool operator==(const IteratorView& first, const IteratorView& second)
        {
            return (first.m_iter == second.m_iter);
        }

        friend bool operator!=(const IteratorView& first, const IteratorView& second)
        {
            return !(first == second);
        }

    private:
        void go_to_closest_valid_item()
        {
            /*поменять местами условия*/
            while ((!m_predicate(*m_iter)) && (m_iter != m_end))
                ++m_iter;
        }

        Predicate m_predicate{};
        Iterator m_iter{};
        Iterator m_end{};
    };


    template <typename Predicate, typename Iterator>
    class Range
    {
    public:
        Range(IteratorView<Predicate, Iterator> begin, IteratorView<Predicate, Iterator> end) : m_begin{begin}, m_end{end} {}
        /*
        Тут надо передавать обычный Iterator и конструировать IteratorView внутри. И потом вохвращать его/
        IteratorView создается только с помощью Range!!!
        */

        IteratorView<Predicate, Iterator> begin() const { return m_begin; }
        IteratorView<Predicate, Iterator> end() const { return m_end; }

    private:
        IteratorView<Predicate, Iterator> m_begin{};
        IteratorView<Predicate, Iterator> m_end{};
    };

}