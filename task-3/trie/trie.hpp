#pragma once

#include <string>
#include <utility>
#include <array>
#include <memory>


namespace ns_Trie
{
    constexpr int alphabet_size = 26;

    template <typename T> class Iterator;


    template <typename T> class ConstIterator;


    template <typename T> class Vertex;


    template <typename T> class SubTrie;


    template <typename T>
    class Trie
    {
    public:
        using iterator = Iterator<T>;
        using const_iterator = ConstIterator<T>;

        using value_type = T;
        using key_type = std::string;

        using size_type = size_t;
    

        Trie()
        {

        }


        template <typename InputIterator>
        Trie(InputIterator first, InputIterator last)
        {

        }


        Trie(const Trie<T>& other)
        {

        }


        Trie<T>& operator=(const Trie<T> other)
        {

        }


        Trie(Trie<T>&& other)
        {

        }


        Trie<T>& operator=(Trie<T>&& other)
        {

        }


        iterator begin();
        const_iterator begin() const;

        iterator end();
        const_iterator end();

        bool empty() const;

        size_type size() const;

        value_type operator[](const key_type& key);

        std::pair<iterator, bool> insert(const key_type& key, const value_type& value);

        template <typename InputIterator>
        void insert(InputIterator first, InputIterator last);

        void erase(iterator position);

        size_type erase(const key_type& key);

        void erase(iterator first, iterator last);

        void swap(Trie<T>& other);

        void clear();

        iterator find(const key_type& key);

        const_iterator find(const key_type& key);

        SubTrie<T> GetSubTrie(const key_type& key);

    private:
        
    };


    template <typename T>
    class SubTrie {};


    /*
    Вопросы:
    1) Храним указатель на объект или сам объект?
    2) Vertex сам создает объект внутри себя или мы передаем ему готовый?
    3) Переделать указатели в умные?
    */
    template <typename T> class Vertex
    {
    public:
        Vertex()
        {
            m_children = {nullptr};
            m_data = nullptr;
            m_end_of_word = false;
        }
        Vertex(const Vertex&) = delete;
        Vertex& operator=(const Vertex&) = delete;
        Vertex(Vertex&&) = delete;
        Vertex& operator=(Vertex&&) = delete;

        T& data() const noexcept { return *m_data; }
        bool end_of_word() const noexcept { return m_end_of_word; }
    
    private:
        // Почему мы вообще можем тут инициализировать поля?
        std::array<Vertex*, alphabet_size> m_children = { nullptr };
        T* m_data = nullptr;
        bool m_end_of_word = false;
    };


    template <typename T>
    class Iterator : public std::iterator<std::forward_iterator_tag, std::pair<std::string, T&>>
    {
    public:
        Iterator(value_type& x);

        Iterator(const Iterator& other);
        Iterator& operator=(const Iterator& other);
        
        Iterator(Iterator&& other);
        Iterator& operator=(Iterator&& other);

        Iterator& operator++();
        Iterator operator++(int);

        bool operator==(const Iterator& other);
        bool operator!=(const Iterator& other);

        value_type operator*();
        value_type* operator->();
    };


    template <typename T>
    class ConstIterator : public std::iterator<std::forward_iterator_tag, std::pair<std::string, T&>>
    {

    };
}
