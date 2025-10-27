#pragma once

#include <iterator>
#include <type_traits>


namespace Filter {

    // template <typename _Iter>
    // struct is_forward_iterator_struct
    // {
    //     using category = typename std::iterator_traits<_Iter>::iterator_category;
    //     static constexpr bool value = std::is_base_of<std::forward_iterator_tag, category>::value;
    // };

    // template <typename _Iter, typename = void>
    // struct is_forward_iterator : std::false_type{};

    // template <typename _Iter>
    // struct is_forward_iterator<_Iter, std::void_t<typename is_forward_iterator_struct<_Iter>::value>> : std::true_type{};

    // template <typename Predicate, typename _Iter, typename = void>
    // class Iterator {};

    template <typename Predicate, typename _Iter,
            typename = std::enable_if_t<
                std::is_base_of_v<
                    std::forward_iterator_tag,
                    typename std::iterator_traits<_Iter>::iterator_category
                >
            >>
    class Iterator
    {
    public:
        using value_type = typename std::iterator_traits<_Iter>::value_type;
        using reference = typename std::iterator_traits<_Iter>::reference;
        using pointer = typename std::iterator_traits<_Iter>::pointer;
        using difference_type = typename std::iterator_traits<_Iter>::difference_type;
        using iterator_category = typename std::forward_iterator_tag;


        Iterator() = default;

        Iterator(Predicate f, _Iter begin, _Iter end) : m_predicate{f}, m_iter{begin}, m_end{end}
        {
            go_to_closest_valid_item();
        }

        Iterator(const Iterator&) = default;
        Iterator& operator=(const Iterator&) = default;

        Iterator(Iterator&&) = default;
        Iterator& operator=(Iterator&&) noexcept = default;

        reference operator*() const { return *m_iter; }

        pointer operator->() const { return &(*m_iter); }

        _Iter iter() const { return m_iter; }
        _Iter end() const { return m_end; }
        Predicate predicate() const { return m_predicate; }


        Iterator& operator++()
        {
            if (m_iter != m_end) ++m_iter;
            go_to_closest_valid_item();
            return *this;
        }

        Iterator operator++(int)
        {
            auto tmp = *this;
            ++*this;
            return tmp;
        }

        friend bool operator==(const Iterator& first, const Iterator& second)
        {
            return (first.m_iter == second.m_iter);
        }

        friend bool operator!=(const Iterator& first, const Iterator& second)
        {
            return !(first == second);
        }

    private:
        void go_to_closest_valid_item()
        {
            while ((m_iter != m_end) && ((!m_predicate(*m_iter))))
                ++m_iter;
        }

        Predicate m_predicate{};
        _Iter m_iter{};
        _Iter m_end{};
    };


    template <typename Predicate, typename _Iter>
    class Range
    {
    public:
        using iterator = Iterator<Predicate, _Iter>;


        Range(Predicate f, _Iter begin, _Iter end) : m_begin{iterator{f, begin, end}}, m_end{iterator{f, end, end}} {}

        iterator begin() const { return m_begin; }
        iterator end() const { return m_end; }

    private:
        iterator m_begin{};
        iterator m_end{};
    };

}