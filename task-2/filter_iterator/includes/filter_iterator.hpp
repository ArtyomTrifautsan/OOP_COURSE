#pragma once

#include <iterator>
#include <type_traits>


template <typename Predicate, typename Iterator, typename = void>
class FilterIterator
{
public:
    typedef std::iterator_traits<Iterator>::value_type value_type;
    typedef std::iterator_traits<Iterator>::reference reference;
    typedef std::iterator_traits<Iterator>::pointer pointer;
    typedef std::iterator_traits<Iterator>::difference_type difference_type;
    typedef std::iterator_traits<Iterator>::iterator_category iterator_category;

    FilterIterator();
    FilterIterator(Predicate f, Iterator x, Iterator end = Iterator());
    FilterIterator(Iterator x, Iterator end = Iterator());

    Predicate predicate() const;
    Iterator end() const;
    Iterator const& base() const;   // это что?
    reference operator*() const;
    FilterIterator& operator++();

private:
    Predicate m_pred;
    Iterator m_iter;
    Iterator m_end;
};



template <typename Predicate, typename Iterator>
class FilterIterator<Predicate, Iterator, std::enable_if_t<std::is_same_v<std::input_iterator_tag, typename std::iterator_traits<Iterator>::iterator_category>>>
{
public:
    typedef std::iterator_traits<Iterator>::value_type value_type;
    typedef std::iterator_traits<Iterator>::reference reference;
    typedef std::iterator_traits<Iterator>::pointer pointer;
    typedef std::iterator_traits<Iterator>::difference_type difference_type;
    typedef std::iterator_traits<Iterator>::iterator_category iterator_category;

    FilterIterator() = default;

    FilterIterator(Predicate f, Iterator x, Iterator end = Iterator()) : m_pred{f}, m_iter{x}, m_end{end}
    {
        go_to_closest_valid_item();
    }

    FilterIterator(Iterator x, Iterator end = Iterator()) : m_pred{}, m_iter{x}, m_end{end} 
    {
        go_to_closest_valid_item();
    }

    Predicate predicate() const { return m_pred; }
    Iterator end() const { return m_end; }
    Iterator const& base() const { return m_iter; }
    reference operator*() const { return *m_iter; }
    FilterIterator& operator++()
    {
        m_iter++;
        go_to_closest_valid_item();
        return *this;
    }

    friend bool operator==(const FilterIterator& first, const FilterIterator& second)
    {
        return (first.m_iter == second.m_iter);
    }

    friend bool operator!=(const FilterIterator& first, const FilterIterator& second)
    {
        return !(first == second);
    }

private:
    void go_to_closest_valid_item()
    {
        while ((!m_pred(*m_iter)) && (m_iter != m_end))
            m_iter++;
    }

    Predicate m_pred{};
    Iterator m_iter{};
    Iterator m_end{};
};


/* Должен ли Filter iterator работать с output_iterator?*/
template <typename Predicate, typename Iterator>
class FilterIterator<Predicate, Iterator, std::enable_if_t<std::is_same_v<std::output_iterator_tag, typename std::iterator_traits<Iterator>::iterator_category>>>;


template <typename Predicate, typename Iterator>
class FilterIterator<Predicate, Iterator, std::enable_if_t<std::is_same_v<std::forward_iterator_tag, typename std::iterator_traits<Iterator>::iterator_category>>>;


template <typename Predicate, typename Iterator>
class FilterIterator<Predicate, Iterator, std::enable_if_t<std::is_same_v<std::bidirectional_iterator_tag, typename std::iterator_traits<Iterator>::iterator_category>>>;


template <typename Predicate, typename Iterator>
class FilterIterator<Predicate, Iterator, std::enable_if_t<std::is_same_v<std::random_access_iterator_tag, typename std::iterator_traits<Iterator>::iterator_category>>>;


template <typename Predicate, typename Iterator>
class FilterIterator<Predicate, Iterator, std::enable_if_t<std::is_same_v<std::contiguous_iterator_tag, typename std::iterator_traits<Iterator>::iterator_category>>>;