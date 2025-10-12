#pragma once

#include <iterator>
#include <type_traits>


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

        IteratorView(Iterator x, Iterator end = Iterator()) : m_predicate{}, m_iter{x}, m_end{end} 
        {
            go_to_closest_valid_item();
        }

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
        // using value_type = std::iterator_traits<Iterator>::value_type;
        // using IteratorType = typename IteratorView<Predicate, Iterator>


        Range(IteratorView<Predicate, Iterator> begin, IteratorView<Predicate, Iterator> end) :
            // m_predicate{f}, 
            m_begin{begin}, 
            m_end{end}
        {
            // Здесь надо доитерироваться до первого item = *m_begin, который f(item) = true.
            // Ну либо до конца, т.е. до m_begin = m_end
        }

        // Predicate predicate() const { return m_predicate; }
        IteratorView<Predicate, Iterator> begin() const { return m_begin; }
        IteratorView<Predicate, Iterator> end() const { return m_end; }

    private:
        // Predicate m_predicate{};
        IteratorView<Predicate, Iterator> m_begin{};
        IteratorView<Predicate, Iterator> m_end{};
    };

}